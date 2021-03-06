#ifndef _INCLUDE_SIGSEGV_PROP_H_
#define _INCLUDE_SIGSEGV_PROP_H_


#include "mem/extract.h"
#include "util/autolist.h"
//#include "util/rtti.h"


/* from src/public/dt_utlvector_send.cpp */
struct CSendPropExtra_UtlVector
{
	SendTableProxyFn m_DataTableProxyFn;	// If it's a datatable, then this is the proxy they specified.
	SendVarProxyFn m_ProxyFn;				// If it's a non-datatable, then this is the proxy they specified.
	EnsureCapacityFn m_EnsureCapacityFn;
	int m_ElementStride;					// Distance between each element in the array.
	int m_Offset;							// # bytes from the parent structure to its utlvector.
	int m_nMaxElements;						// For debugging...
};


namespace Prop
{
	void PreloadAll();
	
	bool FindOffset(int& off, const char *obj, const char *var);
	int FindOffsetAssert(const char *obj, const char *var);
	
//	const datamap_t *GetDataMapByClassname(const char *classname);
//	const datamap_t *GetDataMapByRTTIName(const char *rtti_name);
//	
//	template<typename T> datamap_t *GetDataMapByRTTI()
//	{
//		return GetDataMapByRTTIName(TypeName<T>());
//	}
}


class IProp : public AutoList<IProp>
{
public:
	enum class State : int
	{
		INITIAL,
		OK,
		FAIL,
	};
	
	virtual ~IProp() = default;
	
	virtual const char *GetObjectName() const { return this->m_pszObjName; }
	virtual const char *GetMemberName() const { return this->m_pszMemName; }
	virtual size_t GetSize() const = 0;
	virtual const char *GetKind() const = 0;
	
	void Preload() { this->DoCalcOffset(); }
	
	int GetOffsetAssert();
	bool GetOffset(int& off);
	
	State GetState() const { return this->m_State; }
	
protected:
	IProp(const char *obj, const char *mem) :
		m_pszObjName(obj), m_pszMemName(mem) {}
	
	virtual bool CalcOffset(int& off) const = 0;
	
private:
	void DoCalcOffset();
	
	const char *m_pszObjName;
	const char *m_pszMemName;
	State m_State = State::INITIAL;
	int m_Offset = -1;
};

inline int IProp::GetOffsetAssert()
{
	int off = -1;
	assert(this->GetOffset(off));
	return off;
}

inline bool IProp::GetOffset(int& off)
{
	if (this->m_State == State::INITIAL) {
		this->DoCalcOffset();
	}
	
	if (this->m_State == State::OK) {
		off = this->m_Offset;
		return true;
	} else {
		return false;
	}
}

inline void IProp::DoCalcOffset()
{
	if (this->m_State == State::INITIAL) {
		if (this->CalcOffset(this->m_Offset)) {
			this->m_State = State::OK;
		} else {
			this->m_State = State::FAIL;
		}
	}
}


class CPropBase_SendProp : public IProp
{
public:
	virtual const char *GetKind() const override { return "SENDPROP"; }
	
	void StateChanged(void *obj, void *var)
	{
		(*this->m_pStateChangedFunc)(obj, var);
	}
	
protected:
	CPropBase_SendProp(const char *obj, const char *mem, const char *sv_class, void (*sc_func)(void *, void *), const char *remote_name = nullptr) :
		IProp(obj, mem), m_pszServerClass(sv_class), m_pStateChangedFunc(sc_func), m_pszRemoteName(remote_name) {}
	
	virtual bool CalcOffset(int& off) const override;
	
private:
	ServerClass *FindServerClass() const;
	bool FindSendProp(int& off, SendTable *s_table) const;
	bool IsSendPropUtlVector(int& off, SendProp *q_prop) const;
	const char *GetSendPropMemberName() const;
	
	const char *m_pszServerClass;
	void (*m_pStateChangedFunc)(void *, void *);
	const char *m_pszRemoteName;
};

template<typename T>
class CProp_SendProp final : public CPropBase_SendProp
{
public:
	CProp_SendProp(const char *obj, const char *mem, const char *sv_class, void (*sc_func)(void *, void *), const char *remote_name = nullptr) :
		CPropBase_SendProp(obj, mem, sv_class, sc_func, remote_name) {}
	
	virtual size_t GetSize() const override { return sizeof(T); }
};


class CPropBase_DataMap : public IProp
{
public:
	virtual const char *GetKind() const override { return "DATAMAP"; }
	
	void StateChanged(void *obj, void *var) {}
	
protected:
	CPropBase_DataMap(const char *obj, const char *mem) :
		IProp(obj, mem) {}
	
	virtual bool CalcOffset(int& off) const override;
};

template<typename T>
class CProp_DataMap final : public CPropBase_DataMap
{
public:
	CProp_DataMap(const char *obj, const char *mem) :
		CPropBase_DataMap(obj, mem) {}
	
	virtual size_t GetSize() const override { return sizeof(T); }
};
//#ifdef __GNUC__
//#warning TODO: delete gamedata/sigsegv/datamaps.txt and remove from PackageScript and gameconf.cpp
//#endif


class CPropBase_Extract : public IProp
{
public:
	virtual const char *GetKind() const override { return "EXTRACT"; }
	
	void StateChanged(void *obj, void *var) {}
	
protected:
	CPropBase_Extract(const char *obj, const char *mem) :
		IProp(obj, mem) {}
	
	virtual bool CalcOffset(int& off) const override;
	
	virtual IExtractBase *GetExtractor() const = 0;
};

template<typename T>
class CProp_Extract final : public CPropBase_Extract
{
public:
	CProp_Extract(const char *obj, const char *mem, IExtract<T *> *extractor) :
		CPropBase_Extract(obj, mem), m_Extractor(extractor) {}
	CProp_Extract(const char *obj, const char *mem, IExtractStub *stub) :
		CPropBase_Extract(obj, mem), m_Extractor(nullptr) {}
	
	virtual ~CProp_Extract()
	{
		if (this->m_Extractor != nullptr) {
			delete this->m_Extractor;
		}
	}
	
	virtual size_t GetSize() const override { return sizeof(T); }
	
private:
	virtual bool CalcOffset(int& off) const override
	{
		if (!CPropBase_Extract::CalcOffset(off)) return false;
		
		off = (int)this->m_Extractor->Extract();
		return true;
	}
	
	virtual IExtractBase *GetExtractor() const { return this->m_Extractor; }
	
	IExtract<T *> *m_Extractor;
};


class CPropBase_Relative : public IProp
{
public:
	enum RelativeMethod : int
	{
		REL_MANUAL,
		REL_AFTER,
		REL_BEFORE,
	};
	
	virtual const char *GetKind() const override { return "RELATIVE"; }
	
	void StateChanged(void *obj, void *var) {}
	
protected:
	CPropBase_Relative(const char *obj, const char *mem, IProp *prop, RelativeMethod method, int align, size_t size, int diff = 0) :
		IProp(obj, mem), m_RelProp(prop), m_Method(method), m_iAlign(align), m_iDiff(diff)
	{
		switch (method) {
		default:
		case REL_MANUAL: this->m_iDiff =  diff;                   break;
		case REL_AFTER:  this->m_iDiff =  diff + prop->GetSize(); break;
		case REL_BEFORE: this->m_iDiff = -diff - size;            break;
		}
	}
	
private:
	virtual bool CalcOffset(int& off) const override;
	
	IProp *m_RelProp;
	RelativeMethod m_Method;
	int m_iAlign;
	int m_iDiff;
};

template<typename T>
class CProp_Relative final : public CPropBase_Relative
{
public:
	CProp_Relative(const char *obj, const char *mem, IProp *prop, RelativeMethod method, int align, int diff = 0) :
		CPropBase_Relative(obj, mem, prop, method, align, sizeof(T), diff) {}
	
	virtual size_t GetSize() const override { return sizeof(T); }
};


#define T_PARAMS typename IPROP, IPROP *PROP, const size_t *ADJUST, bool NET, bool RW
#define T_ARGS            IPROP,        PROP,               ADJUST,      NET,      RW


class CPropAccessorBaseBase
{
public:
	
};

template<typename T, T_PARAMS>
class CPropAccessorBase : public CPropAccessorBaseBase
{
private:
	/* determine whether we should be returning writable refs/ptrs */
//	static constexpr bool ReadOnly() { return (NET && !RW); }
	
	/* reference typedefs */
	using RefRO_t = const T&;
	using RefRW_t =       T&;
//	using Ref_t   = std::conditional_t<ReadOnly(), RefRO_t, RefRW_t>;
	using Ref_t   = std::conditional_t<(NET && !RW), RefRO_t, RefRW_t>;
	
	/* pointer typedefs */
	using PtrRO_t = const T*;
	using PtrRW_t =       T*;
//	using Ptr_t   = std::conditional_t<ReadOnly(), PtrRO_t, PtrRW_t>;
	using Ptr_t   = std::conditional_t<(NET && !RW), PtrRO_t, PtrRW_t>;
	
public:
	CPropAccessorBase()                                    = delete;
	CPropAccessorBase(CPropAccessorBase&  copy)            = delete;
	CPropAccessorBase(CPropAccessorBase&& move)            = delete;
	CPropAccessorBase& operator=(CPropAccessorBase&& move) = delete;
	
	/* conversion operators */
	operator Ref_t() const { return this->Get(); }
	
//	template<typename A, bool RW2 = (!NET || RW)> operator std::enable_if_t<( RW2 && std::is_convertible_v<T, A>),       A&>() const { return static_cast<      A&>(this->GetRW()); }
//	template<typename A, bool RW2 = (!NET || RW)> operator std::enable_if_t<(!RW2 && std::is_convertible_v<T, A>), const A&>() const { return static_cast<const A&>(this->GetRO()); }
	
	/* assignment */
	template<typename A> const T& operator=(const CPropAccessorBase<A, T_ARGS>& val) { return this->Set(static_cast<const T>(val.GetRO())); }
	template<typename A> const T& operator=(const A& val)                            { return this->Set(static_cast<const T>(val));         }
	
	/* assignment with modify */
	template<typename A> const T& operator+= (const A& val) { return this->Set(this->GetRO() +  static_cast<const T>(val)); }
	template<typename A> const T& operator-= (const A& val) { return this->Set(this->GetRO() -  static_cast<const T>(val)); }
	template<typename A> const T& operator*= (const A& val) { return this->Set(this->GetRO() *  static_cast<const T>(val)); }
	template<typename A> const T& operator/= (const A& val) { return this->Set(this->GetRO() /  static_cast<const T>(val)); }
	template<typename A> const T& operator%= (const A& val) { return this->Set(this->GetRO() %  static_cast<const T>(val)); }
	template<typename A> const T& operator&= (const A& val) { return this->Set(this->GetRO() &  static_cast<const T>(val)); }
	template<typename A> const T& operator|= (const A& val) { return this->Set(this->GetRO() |  static_cast<const T>(val)); }
	template<typename A> const T& operator^= (const A& val) { return this->Set(this->GetRO() ^  static_cast<const T>(val)); }
	template<typename A> const T& operator<<=(const A& val) { return this->Set(this->GetRO() << static_cast<const T>(val)); }
	template<typename A> const T& operator>>=(const A& val) { return this->Set(this->GetRO() >> static_cast<const T>(val)); }
	
	/* comparison */
	template<typename A> auto operator==(const A& val) const { return (this->GetRO() == val); }
	template<typename A> auto operator!=(const A& val) const { return (this->GetRO() != val); }
	template<typename A> auto operator< (const A& val) const { return (this->GetRO() <  val); }
	template<typename A> auto operator> (const A& val) const { return (this->GetRO() >  val); }
	template<typename A> auto operator<=(const A& val) const { return (this->GetRO() <= val); }
	template<typename A> auto operator>=(const A& val) const { return (this->GetRO() >= val); }
	
	/* unary arithmetic/logic */
	auto operator+() const { return +this->GetRO(); }
	auto operator-() const { return -this->GetRO(); }
	auto operator~() const { return ~this->GetRO(); }
	
	/* binary arithmetic/logic */
	template<typename A> friend auto operator+ (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() +  val        ); }
	template<typename A> friend auto operator+ (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         +  acc.GetRO()); }
	template<typename A> friend auto operator- (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() -  val        ); }
	template<typename A> friend auto operator- (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         -  acc.GetRO()); }
	template<typename A> friend auto operator* (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() *  val        ); }
	template<typename A> friend auto operator* (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         *  acc.GetRO()); }
	template<typename A> friend auto operator/ (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() /  val        ); }
	template<typename A> friend auto operator/ (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         /  acc.GetRO()); }
	template<typename A> friend auto operator% (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() %  val        ); }
	template<typename A> friend auto operator% (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         %  acc.GetRO()); }
	template<typename A> friend auto operator& (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() &  val        ); }
	template<typename A> friend auto operator& (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         &  acc.GetRO()); }
	template<typename A> friend auto operator| (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() |  val        ); }
	template<typename A> friend auto operator| (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         |  acc.GetRO()); }
	template<typename A> friend auto operator^ (const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() ^  val        ); }
	template<typename A> friend auto operator^ (const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         ^  acc.GetRO()); }
	template<typename A> friend auto operator<<(const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() << val        ); }
	template<typename A> friend auto operator<<(const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         << acc.GetRO()); }
	template<typename A> friend auto operator>>(const CPropAccessorBase<T, T_ARGS>& acc, const A& val) { return (acc.GetRO() >> val        ); }
	template<typename A> friend auto operator>>(const A& val, const CPropAccessorBase<T, T_ARGS>& acc) { return (val         >> acc.GetRO()); }
	
	/* operator& */
	Ptr_t operator&() const { return this->GetPtr(); }
	
	/* operator-> */
	Ptr_t operator->() const { return this->GetPtr(); }
	
	/* pre-increment */
	const T& operator++() { return this->Set(this->GetRO() + 1); }
	const T& operator--() { return this->Set(this->GetRO() - 1); }
	
	/* post-increment */
	template<typename T2 = T> std::enable_if_t<!std::is_abstract_v<T2>, T2> operator++(int) { T copy = this->GetRO(); this->Set(this->GetRO() + 1); return copy; }
	template<typename T2 = T> std::enable_if_t<!std::is_abstract_v<T2>, T2> operator--(int) { T copy = this->GetRO(); this->Set(this->GetRO() - 1); return copy; }
	
	/* indexing */
	#ifdef __GNUC__
	//#warning TODO: ensure that operator[] returns reference types and that their constness is correct
	#endif
	template<typename A> inline auto operator[](const A& idx) const { return this->Get()[idx]; }
	
	template<typename A> inline void SetArray(const A val, int index)                            { this->Set(static_cast<const typename std::remove_extent<T>::type>(val),index);}
//	template<typename T2 = T, bool RW2 = (!NET || RW)> typename std::enable_if_t<( RW2 && std::is_array_v<T2>),       T&/* remove extent */> operator[](/* TODO */) const/*?*/ { /* TODO */ }
//	template<typename T2 = T, bool RW2 = (!NET || RW)> typename std::enable_if_t<(!RW2 && std::is_array_v<T2>), const T&/* remove extent */> operator[](/* TODO */) const/*?*/ { /* TODO */ }
//	template<typename T2 = T, bool RW2 = (!NET || RW)> typename std::enable_if_t<( RW2 && std::is_array_v<T2>),       decltype(std::declval<T>()[0])&> operator[](ptrdiff_t idx) const { return this->Get_RW()[idx]; }
//	template<typename T2 = T, bool RW2 = (!NET || RW)> typename std::enable_if_t<(!RW2 && std::is_array_v<T2>), const decltype(std::declval<T>()[0])&> operator[](ptrdiff_t idx) const { return this->Get_RO()[idx]; }
	// only const for now
//	template<typename T2 = T> std::enable_if_t<std::is_function_v<std::declval<T>()[0]>, > 
	// maybe abuse begin() / end() iterator stuff?
	
	/* begin() and end() passthru */
	#ifdef __GNUC__
	//#warning TODO: ensure that begin() and end() return the right things based on constness
	#endif
	auto begin() { return this->Get().begin(); }
	auto end()   { return this->Get().end();   }
	
	/* not implemented yet */
//	template<typename... ARGS> auto operator() (ARGS...) = delete; // TODO
//	template<typename... ARGS> auto operator*  (ARGS...) = delete; // TODO
//	template<typename... ARGS> auto operator->*(ARGS...) = delete; // TODO
	
protected:
	inline RefRO_t Set(RefRO_t val)
	{
		if (NET) {
			if (memcmp(this->GetPtrRO(), &val, sizeof(T)) != 0) {
				PROP->StateChanged(reinterpret_cast<void *>(this->GetInstanceBaseAddr()), this->GetPtrRW());
				return (this->GetRW() = val);
			} else {
				return this->GetRO();
			}
		} else {
			return (this->GetRW() = val);
		}
	}

	inline void Set(typename std::remove_extent<T>::type val, int index)
	{
		if (NET) {
			if (memcmp((this->GetPtrRO()+index), &val, sizeof(typename std::remove_extent<T>::type)) != 0) {
				PROP->StateChanged(reinterpret_cast<void *>(this->GetInstanceBaseAddr()), this->GetPtrRW()+index);
				(this->GetRW()[index] = val);
			} else {
				this->GetRO();
			}
		} else {
			(this->GetRW()[index] = val);
		}
	}

	/* reference getters */
	RefRO_t GetRO() const { return *this->GetPtrRO(); }
	RefRW_t GetRW() const { return *this->GetPtrRW(); }
	Ref_t   Get  () const { return *this->GetPtr  (); }
	
	/* pointer getters */
	PtrRO_t GetPtrRO() const { return reinterpret_cast<PtrRO_t>(this->GetInstanceVarAddr()); }
	PtrRW_t GetPtrRW() const { return reinterpret_cast<PtrRW_t>(this->GetInstanceVarAddr()); }
	Ptr_t   GetPtr  () const { return reinterpret_cast<Ptr_t  >(this->GetInstanceVarAddr()); }
	
private:
	uintptr_t GetInstanceBaseAddr() const { return (reinterpret_cast<uintptr_t>(this) - *ADJUST); }
	uintptr_t GetInstanceVarAddr() const  { return (this->GetInstanceBaseAddr() + this->GetCachedVarOffset()); }
	
	inline ptrdiff_t GetCachedVarOffset() const
	{
		static ptrdiff_t s_CachedVarOff = PROP->GetOffsetAssert();
		return s_CachedVarOff;
	}
};

template<typename U, T_PARAMS>
struct CPropAccessorHandle : public CPropAccessorBase<CHandle<U>, T_ARGS>
{
	using CPropAccessorBase<CHandle<U>, T_ARGS>::operator=;
	
	inline operator U*() const   { return this->GetRO().Get(); }
	inline U* operator->() const { return this->GetRO().Get(); }
};


template<typename T, T_PARAMS>
struct CPropAccessor final : public CPropAccessorBase<T, T_ARGS>
{
	using CPropAccessorBase<T, T_ARGS>::operator=;
};

template<typename U, T_PARAMS>
struct CPropAccessor<CHandle<U>, T_ARGS> final : public CPropAccessorHandle<U, T_ARGS>
{
	using CPropAccessorHandle<U, T_ARGS>::operator=;
};


/* some sanity checks to ensure zero-size, no ctors, no vtable, etc */
#ifdef DEBUG
	#define CHECK_ACCESSOR(ACCESSOR) \
		static_assert( std::is_empty_v                <ACCESSOR>, "NOT GOOD: Prop accessor isn't an empty type"     ); \
		static_assert(!std::is_polymorphic_v          <ACCESSOR>, "NOT GOOD: Prop accessor has virtual functions"   ); \
		static_assert(!std::is_default_constructible_v<ACCESSOR>, "NOT GOOD: Prop accessor is default-constructible"); \
		static_assert(!std::is_copy_constructible_v   <ACCESSOR>, "NOT GOOD: Prop accessor is copy-constructible"   ); \
		static_assert(!std::is_move_constructible_v   <ACCESSOR>, "NOT GOOD: Prop accessor is move-constructible"   )
#else
	#define CHECK_ACCESSOR(ACCESSOR)
#endif


#define DECL_PROP(TYPE, PROPNAME, VARIANT, NET, RW) \
	using _type_prop_##PROPNAME = CProp_##VARIANT<TYPE>; \
	static _type_prop_##PROPNAME s_prop_##PROPNAME; \
	static const size_t _adj_##PROPNAME; \
	using _type_accessor_##PROPNAME = CPropAccessor<TYPE, _type_prop_##PROPNAME, &s_prop_##PROPNAME, &_adj_##PROPNAME, NET, RW>; \
	_type_accessor_##PROPNAME PROPNAME; \
	CHECK_ACCESSOR(_type_accessor_##PROPNAME)

#define DECL_SENDPROP(   TYPE, PROPNAME) DECL_PROP(TYPE, PROPNAME, SendProp, true,  false)
#define DECL_SENDPROP_RW(TYPE, PROPNAME) DECL_PROP(TYPE, PROPNAME, SendProp, true,  true )
#define DECL_DATAMAP(    TYPE, PROPNAME) DECL_PROP(TYPE, PROPNAME, DataMap,  false, false)
#define DECL_EXTRACT(    TYPE, PROPNAME) DECL_PROP(TYPE, PROPNAME, Extract,  false, false)
#define DECL_RELATIVE(   TYPE, PROPNAME) DECL_PROP(TYPE, PROPNAME, Relative, false, false)


// for IMPL_SENDPROP, add an additional argument for the "remote name" (e.g. in CBaseEntity, m_MoveType's remote name is "movetype")
#define IMPL_SENDPROP(TYPE, CLASSNAME, PROPNAME, SVCLASS, ...) \
	void NetworkStateChanged_##CLASSNAME##_##PROPNAME(void *obj, void *var) { reinterpret_cast<CLASSNAME *>(obj)->NetworkStateChanged(var); } \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_SendProp<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, #SVCLASS, &NetworkStateChanged_##CLASSNAME##_##PROPNAME, ##__VA_ARGS__)
#define IMPL_DATAMAP(TYPE, CLASSNAME, PROPNAME) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_DataMap<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME)
#define IMPL_EXTRACT(TYPE, CLASSNAME, PROPNAME, EXTRACTOR) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Extract<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, EXTRACTOR)
#define IMPL_RELATIVE(TYPE, CLASSNAME, PROPNAME, RELPROP, DIFF) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Relative<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, &CLASSNAME::s_prop_##RELPROP, CProp_Relative<TYPE>::REL_MANUAL, 0, DIFF)
#define IMPL_REL_AFTER(TYPE, CLASSNAME, PROPNAME, RELPROP, ...) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Relative<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, &CLASSNAME::s_prop_##RELPROP, CProp_Relative<TYPE>::REL_AFTER, 0, ##__VA_ARGS__)
#define IMPL_REL_BEFORE(TYPE, CLASSNAME, PROPNAME, RELPROP, ...) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Relative<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, &CLASSNAME::s_prop_##RELPROP, CProp_Relative<TYPE>::REL_BEFORE, 0, ##__VA_ARGS__)
#define IMPL_REL_AFTER_ALIGN(TYPE, CLASSNAME, PROPNAME, RELPROP, ALIGN, ...) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Relative<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, &CLASSNAME::s_prop_##RELPROP, CProp_Relative<TYPE>::REL_AFTER, ALIGN, ##__VA_ARGS__)
#define IMPL_REL_BEFORE_ALIGN(TYPE, CLASSNAME, PROPNAME, RELPROP, ALIGN, ...) \
	const size_t CLASSNAME::_adj_##PROPNAME = offsetof(CLASSNAME, PROPNAME); \
	CProp_Relative<TYPE> CLASSNAME::s_prop_##PROPNAME(#CLASSNAME, #PROPNAME, &CLASSNAME::s_prop_##RELPROP, CProp_Relative<TYPE>::REL_BEFORE, ALIGN, ##__VA_ARGS__)


#endif
