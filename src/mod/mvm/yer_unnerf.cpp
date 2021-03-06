#include "mod.h"
#include "prop.h"
#include "stub/econ.h"
#include "stub/tfplayer.h"
#include "stub/tfweaponbase.h"

namespace Mod::MvM::YER_Unnerf
{
	constexpr uint8_t s_Buf[] = {
		0xa1, 0x00, 0x00, 0x00, 0x00,             // +0000  mov eax,[g_pGameRules]
		0x85, 0xc0,                               // +0005  test eax,eax
		0x74, 0x00,                               // +0007  jz +0xAA
		0x80, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, // +0009  cmp byte ptr [eax+m_bPlayingMannVsMachine],0x0
		0x74, 0x00,                               // +0010  jz +0xBB
	};
	
	struct CPatch_CTFKnife_PrimaryAttack : public CPatch
	{
		CPatch_CTFKnife_PrimaryAttack() : CPatch(sizeof(s_Buf)) {}
		
		virtual const char *GetFuncName() const override { return "CTFKnife::PrimaryAttack"; }
		virtual uint32_t GetFuncOffMin() const override  { return 0x0000; }
		virtual uint32_t GetFuncOffMax() const override  { return 0x0800; } // @ 0x0393
		
		virtual bool GetVerifyInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf.CopyFrom(s_Buf);
			
			void *addr__g_pGameRules = AddrManager::GetAddr("g_pGameRules");
			if (addr__g_pGameRules == nullptr) return false;
			
			int off__CTFGameRules_m_bPlayingMannVsMachine;
			if (!Prop::FindOffset(off__CTFGameRules_m_bPlayingMannVsMachine, "CTFGameRules", "m_bPlayingMannVsMachine")) return false;
			
			buf.SetDword(0x00 + 1, (uint32_t)addr__g_pGameRules);
			buf.SetDword(0x09 + 2, (uint32_t)off__CTFGameRules_m_bPlayingMannVsMachine);
			
			mask.SetRange(0x07 + 1, 1, 0x00);
			mask.SetRange(0x10 + 1, 1, 0x00);
			
			return true;
		}
		
		virtual bool GetPatchInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			/* overwrite the CMP instruction with NOPs */
			buf.SetRange(0x09, 7, 0x90);
			
			/* now drop in a 'cmp eax,eax' so the non-MvM mode jump will always occur */
			buf[0x09 + 0] = 0x39;
			buf[0x09 + 1] = 0xc0;
			
			mask.SetRange(0x09, 7, 0xff);
			
			return true;
		}
	};
	
	//Yer nerf: -20% damage vs giants
	DETOUR_DECL_MEMBER(float, CTFKnife_GetMeleeDamage, CBaseEntity *pTarget, int* piDamageType, int* piCustomDamage)
	{
		float ret = DETOUR_MEMBER_CALL(CTFKnife_GetMeleeDamage)(pTarget, piDamageType, piCustomDamage);
		if (*piCustomDamage == TF_DMG_CUSTOM_BACKSTAB && pTarget->IsPlayer() && ToTFPlayer(pTarget)->IsMiniBoss()) {
			auto knife = reinterpret_cast<CTFKnife *>(this);

			int disguise_backstab = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( knife, disguise_backstab, set_disguise_on_backstab);
			
			if (disguise_backstab != 0)
				ret *= 0.9f;
		}
		return ret;
	}

	class CMod : public IMod
	{
	public:
		CMod() : IMod("MvM:YER_Unnerf")
		{
		//	MOD_ADD_DETOUR_MEMBER(CTFKnife_GetMeleeDamage, "CTFKnife::GetMeleeDamage");
			this->AddPatch(new CPatch_CTFKnife_PrimaryAttack());
		}
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_mvm_yer_unnerf", "0", FCVAR_NOTIFY,
		"Mod: remove the MvM-specific nerfs from Your Eternal Reward",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}
