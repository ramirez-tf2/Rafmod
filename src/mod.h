#ifndef _INCLUDE_SIGSEGV_MOD_H_
#define _INCLUDE_SIGSEGV_MOD_H_


#include "mem/patch.h"
#include "mem/detour.h"


class IToggleable
{
public:
	IToggleable(bool start = false) :
		m_bEnabled(start) {}
	virtual ~IToggleable() {}
	
	bool IsEnabled() const { return this->m_bEnabled; }
	
	virtual void Toggle(bool enable)
	{
		if (this->m_bEnabled != enable) {
			if (enable) {
				this->OnEnable();
			} else {
				this->OnDisable();
			}
			
			this->m_bEnabled = enable;
		}
	}
	
	void Enable()  { this->Toggle(true); }
	void Disable() { this->Toggle(false); }
	
protected:
	virtual void OnEnable() = 0;
	virtual void OnDisable() = 0;
	
private:
	bool m_bEnabled;
};


class IHasPatches
{
public:
	virtual ~IHasPatches() {}
	
	virtual const char *GetName() const = 0;
	
	bool LoadPatches();
	void UnloadPatches();
	
	void AddPatch(IPatch *patch);
	
	bool ToggleAllPatches(bool enable);
	bool EnableAllPatches()  { return this->ToggleAllPatches(true); }
	bool DisableAllPatches() { return this->ToggleAllPatches(false); }
	
	size_t GetNumPatches() const     { return this->m_Patches.size(); }
	std::vector<IPatch *>& Patches() { return this->m_Patches; }
	
protected:
	IHasPatches() {}
	
	virtual bool CanAddPatches() const = 0;
	virtual bool CanTogglePatches() const = 0;
	
private:
	std::vector<IPatch *> m_Patches;
};


class IHasDetours
{
public:
	virtual ~IHasDetours() {}
	
	virtual const char *GetName() const = 0;
	
	bool LoadDetours();
	void UnloadDetours();
	
	void AddDetour(IDetour *detour);
	
	bool ToggleDetour(const char *name, bool enable);
	bool EnableDetour(const char *name)  { return this->ToggleDetour(name, true); }
	bool DisableDetour(const char *name) { return this->ToggleDetour(name, false); }
	
	bool ToggleAllDetours(bool enable);
	bool EnableAllDetours()  { return this->ToggleAllDetours(true); }
	bool DisableAllDetours() { return this->ToggleAllDetours(false); }
	
	size_t GetNumDetours() const                 { return this->m_Detours.size(); }
	std::map<const char *, IDetour *>& Detours() { return this->m_Detours; }
	
protected:
	IHasDetours() {}
	
	virtual bool CanAddDetours() const = 0;
	virtual bool CanToggleDetours() const = 0;
	
private:
	std::map<const char *, IDetour *> m_Detours;
};


class IMod : public AutoList<IMod>, public IToggleable, public IHasPatches, public IHasDetours
{
public:
	virtual ~IMod() {}
	
	virtual const char *GetName() const override final { return this->m_pszName; }
	
	virtual void Toggle(bool enable) override;
	
protected:
	IMod(const char *name) :
		IToggleable(false), m_pszName(name) {}
	
	virtual void OnEnable() override  {}
	virtual void OnDisable() override {}
	
	// for e.g. dynamically adding patches/detours: ctor is too early, but OnLoad is too late
	virtual void PreLoad() {}
	
	virtual bool OnLoad()   { return true; }
	virtual void OnUnload() {}
	
	virtual bool EnableByDefault() { return false; }
	
private:
	void InvokeLoad();
	void InvokeUnload();
	
	virtual bool CanAddPatches() const override    { return !this->m_bLoaded; }
	virtual bool CanTogglePatches() const override { return !this->m_bFailed; }
	
	virtual bool CanAddDetours() const override    { return !this->m_bLoaded; }
	virtual bool CanToggleDetours() const override { return !this->m_bFailed; }
	
	const char *m_pszName;
	
	bool m_bFailed = false;
	bool m_bLoaded = false;
	
	friend class CModManager;
};


class CModManager : public CBaseGameSystemPerFrame
{
public:
	void Load();
	void Unload();
	
	static void CC_ListMods(const CCommand& cmd);
	
private:
	// CBaseGameSystemPerFrame
	virtual const char *Name() override { return "CModManager"; }
	virtual void LevelInitPreEntity() override;
	virtual void LevelInitPostEntity() override;
	virtual void LevelShutdownPreEntity() override;
	virtual void LevelShutdownPostEntity() override;
	virtual void FrameUpdatePreEntityThink() override;
	virtual void FrameUpdatePostEntityThink() override;
	virtual void PreClientUpdate() override;
};
extern CModManager g_ModManager;


class IModCallbackListener : public AutoList<IModCallbackListener>
{
public:
	virtual bool ShouldReceiveCallbacks() const = 0;
	
	virtual void LevelInitPreEntity() {}
	virtual void LevelInitPostEntity() {}
	
	virtual void LevelShutdownPreEntity() {}
	virtual void LevelShutdownPostEntity() {}
	
	// NOTE: these frame-based callbacks are for the SERVER SIDE only; see IGameSystemPerFrame definition for details!
	
protected:
	IModCallbackListener() {}
};

// DEPRECATED: use IModCallbackListener instead!
class IFrameUpdatePreEntityThinkListener : public AutoList<IFrameUpdatePreEntityThinkListener>
{
public:
	virtual bool ShouldReceiveCallbacks() const = 0;

	virtual void FrameUpdatePreEntityThink() {}
protected:
	IFrameUpdatePreEntityThinkListener() {}
};

class IFrameUpdatePostEntityThinkListener : public AutoList<IFrameUpdatePostEntityThinkListener>
{
public:
	virtual bool ShouldReceiveCallbacks() const = 0;

	virtual void FrameUpdatePostEntityThink() {}
protected:
	IFrameUpdatePostEntityThinkListener() {}
};

class IPreClientUpdateListener : public AutoList<IPreClientUpdateListener>
{
public:
	virtual bool ShouldReceiveCallbacks() const = 0;

	virtual void PreClientUpdate() {}
protected:
	IPreClientUpdateListener() {}
};

#define MOD_ADD_DETOUR_MEMBER(detour, addr) \
	this->AddDetour(new CDetour(addr, GET_MEMBER_CALLBACK(detour), GET_MEMBER_INNERPTR(detour)))
#define MOD_ADD_DETOUR_STATIC(detour, addr) \
	this->AddDetour(new CDetour(addr, GET_STATIC_CALLBACK(detour), GET_STATIC_INNERPTR(detour)))


#endif
