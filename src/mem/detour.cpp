#include "mem/detour.h"
#include "addr/addr.h"
#include "mem/alloc.h"
#include "mem/protect.h"
#include "mem/opcode.h"
#include "mem/wrapper.h"
#include "util/backtrace.h"
#include "util/demangle.h"

#include <udis86.h>

#include <regex>


#if !(defined(__i386) || defined(_M_IX86))
#error Architecture must be IA32
#endif


//#define DETOUR_DEBUG 1


#if DETOUR_DEBUG
#define TRACE_ENABLE 1
#define TRACE_TERSE  1
#endif
#include "util/trace.h"


/* get number of instruction operands */
static unsigned int UD86_num_operands(struct ud *ud)
{
	for (unsigned int i = 0; i < 4; ++i) {
		if (ud_insn_opr(ud, i) == nullptr) return i;
	}
	return 4;
}


/* detect instruction: '(jmp|call) <rel_imm32>' */
static bool UD86_insn_is_jmpcall_rel_imm32(struct ud *ud, uint8_t *opcode_byte = nullptr, int32_t *rel_offset = nullptr)
{
	auto mnemonic = ud_insn_mnemonic(ud);
	if (mnemonic != UD_Ijmp && mnemonic != UD_Icall) return false;
	
	if (ud_insn_len(ud) != 5)       return false;
	if (UD86_num_operands(ud) != 1) return false;
	
	const auto *op0 = ud_insn_opr(ud, 0);
	if (op0->type != UD_OP_JIMM) return false;
	if (op0->size != 32)         return false;
	
	/* optional parameter: write out the main opcode byte */
	if (opcode_byte != nullptr) {
		*opcode_byte = ud_insn_ptr(ud)[0];
	}
	
	/* optional parameter: write out the relative jmp/call offset */
	if (rel_offset != nullptr) {
		*rel_offset = op0->lval.sdword;
	}
	
	return true;
}

/* detect instruction: 'call <rel_imm32>' */
static bool UD86_insn_is_call_rel_imm32(struct ud *ud, const uint8_t **call_target = nullptr)
{
	auto mnemonic = ud_insn_mnemonic(ud);
	if (mnemonic != UD_Icall) return false;
	
	if (ud_insn_len(ud) != 5)       return false;
	if (UD86_num_operands(ud) != 1) return false;
	
	const auto *op0 = ud_insn_opr(ud, 0);
	if (op0->type != UD_OP_JIMM) return false;
	if (op0->size != 32)         return false;
	
	/* optional parameter: write out the call destination address */
	if (call_target != nullptr) {
		*call_target = (const uint8_t *)(ud_insn_off(ud) + ud_insn_len(ud) + op0->lval.udword);
	}
	
	return true;
}

/* detect instruction: 'mov e[acdb]x,[esp]' */
static bool UD86_insn_is_mov_r32_rtnval(struct ud *ud, X86Instr::Reg *dest_reg = nullptr)
{
	auto mnemonic = ud_insn_mnemonic(ud);
	if (mnemonic != UD_Imov) return false;
	
	if (ud_insn_len(ud) != 3)       return false;
	if (UD86_num_operands(ud) != 2) return false;
	
	const auto *op0 = ud_insn_opr(ud, 0);
	if (op0->type != UD_OP_REG) return false;
	if (op0->size != 32)        return false;
	
	X86Instr::Reg reg;
	switch (op0->base) {
	case UD_R_EAX: reg = X86Instr::REG_AX; break;
	case UD_R_ECX: reg = X86Instr::REG_CX; break;
	case UD_R_EDX: reg = X86Instr::REG_DX; break;
	case UD_R_EBX: reg = X86Instr::REG_BX; break;
	default: return false;
	}
	
	const auto *op1 = ud_insn_opr(ud, 1);
	if (op1->type   != UD_OP_MEM) return false;
	if (op1->size   != 32)        return false;
	if (op1->base   != UD_R_ESP)  return false;
	if (op1->index  != UD_NONE)   return false;
	if (op1->scale  != UD_NONE)   return false;
	if (op1->offset != 0)         return false;
	
	/* optional parameter: write out the first operand base register */
	if (dest_reg != nullptr) {
		*dest_reg = reg;
	}
	
	return true;
}

/* detect instruction: 'ret' */
static bool UD86_insn_is_ret(struct ud *ud)
{
	auto mnemonic = ud_insn_mnemonic(ud);
	if (mnemonic != UD_Iret) return false;
	
	if (ud_insn_len(ud) != 1)       return false;
	if (UD86_num_operands(ud) != 0) return false;
	
	return true;
}


/* detect whether an instruction is a call to __i686.get_pc_thunk.(ax|cx|dx|bx) */
static bool UD86_insn_is_call_to_get_pc_thunk(struct ud *ud, X86Instr::Reg *dest_reg = nullptr)
{
	const uint8_t *call_target;
	if (!UD86_insn_is_call_rel_imm32(ud, &call_target)) return false;
	
	ud_t ux;
	ud_init(&ux);
	ud_set_mode(&ux, 32);
	ud_set_pc(&ux, (uint64_t)call_target);
	ud_set_input_buffer(&ux, call_target, 0x100);
	
	if (ud_decode(&ux) == 0) return false;
	if (!UD86_insn_is_mov_r32_rtnval(&ux, dest_reg)) return false;
	
	if (ud_decode(&ux) == 0) return false;
	if (!UD86_insn_is_ret(&ux)) return false;
	
	return true;
}


/* analogous to asm.c copy_bytes() when dest == nullptr */
static size_t Trampoline_CalcNumBytesToCopy(size_t len_min, const uint8_t *func)
{
	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 32);
	ud_set_pc(&ud, (uint64_t)func);
	ud_set_input_buffer(&ud, func, 0x100);
	
	size_t len_actual = 0;
	while (len_actual < len_min) {
		size_t len_decoded = ud_decode(&ud);
		assert(len_decoded != 0);
		len_actual += len_decoded;
	}
	return len_actual;
}

/* analogous to asm.c copy_bytes() when dest != nullptr */
static size_t Trampoline_CopyAndFixUpFuncBytes(size_t len_min, const uint8_t *func, uint8_t *trampoline)
{
	uint8_t *dest = trampoline;
	
	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 32);
	ud_set_pc(&ud, (uint64_t)func);
	ud_set_input_buffer(&ud, func, 0x100);
	
	size_t len_actual = 0;
	while (len_actual < len_min) {
		size_t len_decoded = ud_decode(&ud);
		assert(len_decoded != 0);
		len_actual += len_decoded;
		
		/* detect calls to __i686.get_pc_thunk.(ax|cx|dx|bx);
		 * convert them into direct-register-load operations */
		X86Instr::Reg reg;
		if (UD86_insn_is_call_to_get_pc_thunk(&ud, &reg)) {
			uint32_t pc_value = (ud_insn_off(&ud) + ud_insn_len(&ud) + (trampoline - func));
			MovRegImm32(dest, reg, pc_value).Write();
		} else {
			/* fixup jmp and call relative offsets */
			uint8_t opcode_byte;
			int32_t rel_offset;
			if (UD86_insn_is_jmpcall_rel_imm32(&ud, &opcode_byte, &rel_offset)) {
				rel_offset += (trampoline - func);
				*dest = opcode_byte;
				*reinterpret_cast<int32_t *>(dest + 1) = rel_offset;
			} else {
				memcpy(dest, (uint8_t *)ud_insn_off(&ud), len_decoded);
			}
		}
		
		dest += len_decoded;
	}
	return len_actual;
}
#warning ALL OF THIS NEEDS TESTING!


bool IDetour::Load()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	if (this->DoLoad()) {
		this->m_bLoaded = true;
		return true;
	}
	
	return false;
}

void IDetour::Unload()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	this->Disable();
	this->DoUnload();
}


void IDetour::Toggle(bool enable)
{
	if (enable) {
		this->Enable();
	} else {
		this->Disable();
	}
}

void IDetour::Enable()
{
	TRACE("[this: %08x \"%s\"] loaded:%c active:%c", (uintptr_t)this, this->GetName(),
		(this->IsLoaded() ? 'Y' : 'N'),
		(this->IsActive() ? 'Y' : 'N'));
	
	if (this->IsLoaded() && !this->IsActive()) {
		this->DoEnable();
		this->m_bActive = true;
	}
}

void IDetour::Disable()
{
	TRACE("[this: %08x \"%s\"] loaded:%c active:%c", (uintptr_t)this, this->GetName(),
		(this->IsLoaded() ? 'Y' : 'N'),
		(this->IsActive() ? 'Y' : 'N'));
	
	if (this->IsLoaded() && this->IsActive()) {
		this->DoDisable();
		this->m_bActive = false;
	}
}


void IDetour::DoEnable()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	ITrace *trace   = nullptr;
	CDetour *detour = nullptr;
	
	if ((trace = dynamic_cast<ITrace *>(this)) != nullptr) {
		CDetouredFunc::Find(this->GetFuncPtr()).AddTrace(trace);
	} else if ((detour = dynamic_cast<CDetour *>(this)) != nullptr) {
		CDetouredFunc::Find(this->GetFuncPtr()).AddDetour(detour);
	} else {
		/* don't know how to deal with this kind of IDetour */
		assert(false);
	}
}

void IDetour::DoDisable()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	ITrace *trace   = nullptr;
	CDetour *detour = nullptr;
	
	if ((trace = dynamic_cast<ITrace *>(this)) != nullptr) {
		CDetouredFunc::Find(this->GetFuncPtr()).RemoveTrace(trace);
	} else if ((detour = dynamic_cast<CDetour *>(this)) != nullptr) {
		CDetouredFunc::Find(this->GetFuncPtr()).RemoveDetour(detour);
	} else {
		/* don't know how to deal with this kind of IDetour */
		assert(false);
	}
}


bool IDetour_SymNormal::DoLoad()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	if (this->m_bFuncByName) {
		this->m_pFunc = reinterpret_cast<uint8_t *>(AddrManager::GetAddr(this->m_strFuncName.c_str()));
		if (this->m_pFunc == nullptr) {
			DevMsg("IDetour_SymNormal::DoLoad: \"%s\": addr lookup failed for \"%s\"\n", this->GetName(), this->m_strFuncName.c_str());
			return false;
		}
	} else {
		if (this->m_pFunc == nullptr) {
			DevMsg("IDetour_SymNormal::DoLoad: \"%s\": func ptr provided is nullptr\n", this->GetName());
			return false;
		}
	}
	
	return true;
}

void IDetour_SymNormal::DoUnload()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
}


const char *IDetour_SymRegex::GetName() const
{
	if (this->IsLoaded()) {
		return this->m_strDemangled.c_str();
	} else {
		return this->m_strPattern.c_str();
	}
}


bool IDetour_SymRegex::DoLoad()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
	
	if (this->m_Library == Library::INVALID) {
		DevMsg("IDetour_SymRegex::DoLoad: \"%s\": invalid library\n", this->GetName());
		return false;
	}
	
	if (!LibMgr::HaveLib(this->m_Library)) {
		DevMsg("IDetour_SymRegex::DoLoad: \"%s\": library not available: %s\n", this->GetName(), LibMgr::Lib_ToString(this->m_Library));
		return false;
	}
	
	const SegInfo& info_seg_text = LibMgr::GetInfo(this->m_Library).GetSeg(Segment::TEXT);
	auto text_begin = reinterpret_cast<const void *>(info_seg_text.AddrBegin());
	auto text_end   = reinterpret_cast<const void *>(info_seg_text.AddrEnd());
	
#ifndef _MSC_VER
	#warning NEED try/catch for std::regex ctor!
#endif
	std::regex filter(this->m_strPattern, std::regex_constants::ECMAScript);
	std::vector<Symbol> syms;
	LibMgr::ForEachSym(this->m_Library, [&](const Symbol& sym){
		if (std::regex_search(sym.name, filter, std::regex_constants::match_any)) {
			if (sym.addr >= text_begin && sym.addr < text_end) {
				syms.push_back(sym);
			}
		}
		
		return true;
	});
	
	if (syms.size() != 1) {
		DevMsg("IDetour_SymRegex::DoLoad: \"%s\": symbol lookup failed (%zu matches):\n", this->GetName(), syms.size());
		for (const auto& sym : syms) {
			DevMsg("  %s\n", sym.name.c_str());
		}
		return false;
	}
	
	this->m_strSymbol = syms[0].name;
	this->m_pFunc     = reinterpret_cast<uint8_t *>(syms[0].addr);
	
	DemangleName(this->m_strSymbol.c_str(), this->m_strDemangled);
	
	return true;
}

void IDetour_SymRegex::DoUnload()
{
	TRACE("[this: %08x \"%s\"]", (uintptr_t)this, this->GetName());
}


bool CDetour::DoLoad()
{
	if (!IDetour_SymNormal::DoLoad()) {
		return false;
	}
	
	if (!this->EnsureUniqueInnerPtrs()) {
		return false;
	}
	
	s_LoadedDetours.push_back(this);
	return true;
}

void CDetour::DoUnload()
{
	IDetour_SymNormal::DoUnload();
	
	s_LoadedDetours.erase(std::remove(s_LoadedDetours.begin(), s_LoadedDetours.end(), this), s_LoadedDetours.end());
}


void CDetour::DoEnable()
{
	IDetour::DoEnable();
	
	s_ActiveDetours.push_back(this);
}

void CDetour::DoDisable()
{
	IDetour::DoDisable();
	
	s_ActiveDetours.erase(std::remove(s_ActiveDetours.begin(), s_ActiveDetours.end(), this), s_ActiveDetours.end());
}


/* ensure that no two loaded CDetour instances have the same m_pInner value:
 * this is important because it may seem like two different function detours can
 * share the same callback; but in fact, that's completely unsafe */
bool CDetour::EnsureUniqueInnerPtrs()
{
	TRACE("[this: %08x \"%s\"] %08x", (uintptr_t)this, this->GetName(), (uintptr_t)this->m_pInner);
	
	for (auto detour : s_LoadedDetours) {
		if (detour == this) continue;
		if (detour->m_pInner == this->m_pInner) {
			Warning("Found two CDetour instances with the same inner function pointer!\n"
				"this: %08x \"%s\"\n"
				"that: %08x \"%s\"\n",
				(uintptr_t)this, this->GetName(),
				(uintptr_t)detour, detour->GetName());
			return false;
		}
	}
	
	return true;
}


void CFuncCount::TracePre()
{
	++this->m_RefCount;
}

void CFuncCount::TracePost()
{
	--this->m_RefCount;
}


void CFuncCallback::TracePre()
{
	if (this->m_pCBPre != nullptr) {
		this->m_pCBPre();
	}
}

void CFuncCallback::TracePost()
{
	if (this->m_pCBPost != nullptr) {
		this->m_pCBPost();
	}
}


void CFuncTrace::TracePre()
{
	ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "[%7d] %*s%s ", gpGlobals->tickcount, 2 * TraceLevel::Get(), "", "{");
	ConColorMsg(Color(0x00, 0xff, 0xff, 0xff), "%s\n", this->GetName());
	TraceLevel::Increment();
}

void CFuncTrace::TracePost()
{
	TraceLevel::Decrement();
	ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "[%7d] %*s%s ", gpGlobals->tickcount, 2 * TraceLevel::Get(), "", "}");
	ConColorMsg(Color(0x00, 0xff, 0xff, 0xff), "%s\n", this->GetName());
}


void CFuncBacktrace::TracePre()
{
	BACKTRACE();
}

void CFuncBacktrace::TracePost()
{
}


void CFuncVProf::TracePre()
{
	if (g_VProfCurrentProfile.IsEnabled()) {
		g_VProfCurrentProfile.EnterScope(this->m_strVProfName.c_str(), 0,
			this->m_strVProfGroup.c_str(), false, BUDGETFLAG_OTHER);
	}
}

void CFuncVProf::TracePost()
{
	if (g_VProfCurrentProfile.IsEnabled()) {
		g_VProfCurrentProfile.ExitScope();
	}
}


CDetouredFunc::CDetouredFunc(void *func_ptr) :
	m_pFunc(reinterpret_cast<uint8_t *>(func_ptr))
{
	TRACE("[this: %08x] [func: %08x]", (uintptr_t)this, (uintptr_t)func_ptr);
	
	this->CreateWrapper();
	this->CreateTrampoline();
}
CDetouredFunc::~CDetouredFunc()
{
	TRACE("[this: %08x] [func: %08x]", (uintptr_t)this, (uintptr_t)this->m_pFunc);
	
	this->RemoveAllDetours();
	this->DestroyWrapper();
	this->DestroyTrampoline();
}


CDetouredFunc& CDetouredFunc::Find(void *func_ptr)
{
//	TRACE("%08x", (uintptr_t)func_ptr);
	
	auto it = s_FuncMap.find(func_ptr);
	if (it == s_FuncMap.end()) {
		/* oh god C++11 why do you do this to me */
		auto result = s_FuncMap.emplace(std::piecewise_construct, std::forward_as_tuple(func_ptr), std::forward_as_tuple(func_ptr));
		it = result.first;
	}
	
	return (*it).second;
	
//	TRACE_EXIT("%s", (result.second ? "inserted" : "existing"));
}


void CDetouredFunc::CleanUp()
{
	for (auto it = s_FuncMap.begin(); it != s_FuncMap.end(); ) {
		const CDetouredFunc& df = (*it).second;
		if (df.m_Detours.empty() && df.m_Traces.empty()) {
			it = s_FuncMap.erase(it);
		} else {
			++it;
		}
	}
}


void CDetouredFunc::AddDetour(CDetour *detour)
{
	TRACE("[this: %08x] [detour: %08x \"%s\"]", (uintptr_t)this, (uintptr_t)detour, detour->GetName());
	
	assert(std::count(this->m_Detours.begin(), this->m_Detours.end(), detour) == 0);
	this->m_Detours.push_back(detour);
	
	this->Reconfigure();
}

void CDetouredFunc::RemoveDetour(CDetour *detour)
{
	TRACE("[this: %08x] [detour: %08x \"%s\"]", (uintptr_t)this, (uintptr_t)detour, detour->GetName());
	
	assert(std::count(this->m_Detours.begin(), this->m_Detours.end(), detour) == 1);
	this->m_Detours.erase(std::remove(this->m_Detours.begin(), this->m_Detours.end(), detour), this->m_Detours.end());
	
	this->Reconfigure();
}


void CDetouredFunc::AddTrace(ITrace *trace)
{
	TRACE("[this: %08x] [trace: %08x \"%s\"]", (uintptr_t)this, (uintptr_t)trace, trace->GetName());
	
	assert(std::count(this->m_Traces.begin(), this->m_Traces.end(), trace) == 0);
	this->m_Traces.push_back(trace);
	
	this->Reconfigure();
}

void CDetouredFunc::RemoveTrace(ITrace *trace)
{
	TRACE("[this: %08x] [trace: %08x \"%s\"]", (uintptr_t)this, (uintptr_t)trace, trace->GetName());
	
	assert(std::count(this->m_Traces.begin(), this->m_Traces.end(), trace) == 1);
	this->m_Traces.erase(std::remove(this->m_Traces.begin(), this->m_Traces.end(), trace), this->m_Traces.end());
	
	this->Reconfigure();
}


void CDetouredFunc::RemoveAllDetours()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
	this->m_Detours.clear();
	this->m_Traces.clear();
	
	this->Reconfigure();
}


void CDetouredFunc::CreateWrapper()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
#if !defined _WINDOWS
	this->m_pWrapper = TheExecMemManager()->AllocWrapper();
	{
		MemProtModifier_RX_RWX(this->m_pWrapper, Wrapper::Size());
		
		memcpy(this->m_pWrapper, Wrapper::Base(), Wrapper::Size());
		
		auto p_mov_funcaddr_1 = this->m_pWrapper + Wrapper::Offset_MOV_FuncAddr_1();
		auto p_call_pre       = this->m_pWrapper + Wrapper::Offset_CALL_Pre();
		auto p_call_inner     = this->m_pWrapper + Wrapper::Offset_CALL_Inner();
		auto p_mov_funcaddr_2 = this->m_pWrapper + Wrapper::Offset_MOV_FuncAddr_2();
		auto p_call_post      = this->m_pWrapper + Wrapper::Offset_CALL_Post();
		
		MovRegImm32      (p_mov_funcaddr_1, X86Instr::REG_CX, (uint32_t)this->m_pFunc)         .Write();
		CallIndirectMem32(p_call_pre,                         (uint32_t)&this->m_pWrapperPre)  .Write();
		CallIndirectMem32(p_call_inner,                       (uint32_t)&this->m_pWrapperInner).Write();
		MovRegImm32      (p_mov_funcaddr_2, X86Instr::REG_CX, (uint32_t)this->m_pFunc)         .Write();
		CallIndirectMem32(p_call_post,                        (uint32_t)&this->m_pWrapperPost) .Write();
	}
#endif
}

void CDetouredFunc::DestroyWrapper()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
#if !defined _WINDOWS
	if (this->m_pWrapper != nullptr) {
		TheExecMemManager()->FreeWrapper(this->m_pWrapper);
		this->m_pWrapper = nullptr;
	}
#endif
}


void CDetouredFunc::CreateTrampoline()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
	size_t len_prologue = Trampoline_CalcNumBytesToCopy(JmpRelImm32::Size(), this->m_pFunc);
	TRACE_MSG("len_prologue = %zu\n", len_prologue);
	assert(len_prologue >= JmpRelImm32::Size());
	
	size_t len_trampoline = len_prologue + JmpRelImm32::Size();
	TRACE_MSG("len_trampoline = %zu\n", len_trampoline);
	
	this->m_pTrampoline = TheExecMemManager()->AllocTrampoline(len_trampoline);
	TRACE_MSG("trampoline @ %08x\n", (uintptr_t)this->m_pTrampoline);
	
	assert(!this->IsPrologueValid());
	this->m_Prologue.resize(len_prologue);
	memcpy(this->m_Prologue.data(), this->m_pFunc, len_prologue);
	
	{
		MemProtModifier_RX_RWX(this->m_pTrampoline, len_trampoline);
		assert(Trampoline_CopyAndFixUpFuncBytes(JmpRelImm32::Size(), this->m_pFunc, this->m_pTrampoline) == len_prologue);
		JmpRelImm32(this->m_pTrampoline + len_prologue, (uint32_t)this->m_pFunc + len_prologue).Write();
	}
}

void CDetouredFunc::DestroyTrampoline()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
	if (this->m_pTrampoline != nullptr) {
		TheExecMemManager()->FreeTrampoline(this->m_pTrampoline);
		this->m_pTrampoline = nullptr;
	}
}


void CDetouredFunc::Reconfigure()
{
	TRACE("[this: %08x] with %zu detour(s)", (uintptr_t)this, this->m_Detours.size());
	
	this->UninstallJump();
	
	void *jump_to = nullptr;
	
	if (!this->m_Detours.empty()) {
		CDetour *first = this->m_Detours.front();
		CDetour *last  = this->m_Detours.back();
		
		TRACE_MSG("detour[\"%s\"].inner [%08x] -> trampoline [%08x]\n",
			last->GetName(), (uintptr_t)last->m_pInner, (uintptr_t)this->m_pTrampoline);
		*last->m_pInner = this->m_pTrampoline;
		
		for (int i = this->m_Detours.size() - 2; i >= 0; --i) {
			CDetour *d1 = this->m_Detours[i];
			CDetour *d2 = this->m_Detours[i + 1];
			
			TRACE_MSG("detour[\"%s\"].inner [%08x] -> detour[\"%s\"].callback [%08x]\n",
				d1->GetName(), (uintptr_t)d1->m_pInner,
				d2->GetName(), (uintptr_t)d2->m_pCallback);
			*d1->m_pInner = d2->m_pCallback;
		}
		
		/* just to make sure... */
		for (auto detour : this->m_Detours) {
			assert(detour->EnsureUniqueInnerPtrs());
		}
		
		jump_to = first->m_pCallback;
		
		/*TRACE_MSG("func jump -> detour[\"%s\"].callback [%08x]\n",
			first->GetName(), (uintptr_t)first->m_pCallback);
		this->InstallJump(first->m_pCallback);*/
	}
	
#if !defined _WINDOWS
	if (!this->m_Traces.empty()) {
		if (jump_to != nullptr) {
			this->m_pWrapperInner = jump_to;
		} else {
			this->m_pWrapperInner = this->m_pTrampoline;
		}
		
		jump_to = this->m_pWrapper;
	}
#endif
	
	if (jump_to != nullptr) {
		this->InstallJump(jump_to);
	}
}


void CDetouredFunc::InstallJump(void *target)
{
	TRACE("[this: %08x] [target: %08x]", (uintptr_t)this, (uintptr_t)target);
	
	{
		MemProtModifier_RX_RWX(this->m_pFunc, this->m_Prologue.size());
		JmpRelImm32(this->m_pFunc, (uint32_t)target).WritePadded(this->m_Prologue.size());
	}
}

void CDetouredFunc::UninstallJump()
{
	TRACE("[this: %08x]", (uintptr_t)this);
	
	assert(this->IsPrologueValid());
	
	{
		MemProtModifier_RX_RWX(this->m_pFunc, this->m_Prologue.size());
		memcpy(this->m_pFunc, this->m_Prologue.data(), this->m_Prologue.size());
	}
}


#if !defined _WINDOWS

void CDetouredFunc::FuncPre()
{
	for (auto trace : this->m_Traces) {
		trace->TracePre();
	}
}

void CDetouredFunc::FuncPost()
{
	for (auto trace : this->m_Traces) {
		trace->TracePost();
	}
}


__fastcall void CDetouredFunc::WrapperPre(void *func_ptr, const uint32_t *retaddr_save)
{
//	DevMsg("WrapperPre [func: %08x] [retaddr_save: %08x]\n", (uintptr_t)func_ptr, (uintptr_t)retaddr_save);
	
	/* use the func_ptr arg to locate the correct CDetouredFunc instance */
	auto it = s_FuncMap.find(func_ptr);
	assert(it != s_FuncMap.end());
	CDetouredFunc& inst = (*it).second;
	
	/* do the pre-function stuff for this function (traces etc) */
	inst.FuncPre();
	
	/* save the wrapper's return address, for restoration later */
	s_WrapperCallerRetAddrs.push(*retaddr_save);
}

__fastcall void CDetouredFunc::WrapperPost(void *func_ptr, uint32_t *retaddr_restore)
{
//	DevMsg("WrapperPost [func: %08x] [retaddr_restore: %08x]\n", (uintptr_t)func_ptr, (uintptr_t)retaddr_restore);
	
	/* restore the wrapper's return address, which we saved earlier */
	assert(!s_WrapperCallerRetAddrs.empty());
	*retaddr_restore = s_WrapperCallerRetAddrs.top();
	s_WrapperCallerRetAddrs.pop();
	
	/* use the func_ptr arg to locate the correct CDetouredFunc instance */
	auto it = s_FuncMap.find(func_ptr);
	assert(it != s_FuncMap.end());
	CDetouredFunc& inst = (*it).second;
	
	/* do the post-function stuff for this function (traces etc) */
	inst.FuncPost();
}

#endif
