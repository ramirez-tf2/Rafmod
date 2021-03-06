#ifndef _INCLUDE_SIGSEGV_STUB_MISC_H_
#define _INCLUDE_SIGSEGV_STUB_MISC_H_


#include "link/link.h"
#include "stub/usermessages_sv.h"
#include "stub/tfplayer.h"


class CTFBotMvMEngineerTeleportSpawn;
class CTFBotMvMEngineerBuildTeleportExit;
class CTFBotMvMEngineerBuildSentryGun;

class CFlaggedEntitiesEnum;

class CRConClient;

class CBaseEntity;
class CBasePlayer;


CRConClient& RCONClient();

typedef struct hudtextparms_s
{
	float		x;
	float		y;
	int			effect;
	byte		r1, g1, b1, a1;
	byte		r2, g2, b2, a2;
	float		fadeinTime;
	float		fadeoutTime;
	float		holdTime;
	float		fxTime;
	int			channel;
} hudtextparms_t;

#define DECL_FT_WRAPPER(name) \
	template<typename... ARGS> \
	decltype(ft_##name)::RetType name(ARGS&&... args) \
	{ return ft_##name(std::forward<ARGS>(args)...); }


extern StaticFuncThunk<void, const Vector&, trace_t&, const Vector&, const Vector&, CBaseEntity *> ft_FindHullIntersection;
DECL_FT_WRAPPER(FindHullIntersection);


extern StaticFuncThunk<int> ft_UTIL_GetCommandClientIndex;
DECL_FT_WRAPPER(UTIL_GetCommandClientIndex);

extern StaticFuncThunk<CBasePlayer *> ft_UTIL_GetCommandClient;
DECL_FT_WRAPPER(UTIL_GetCommandClient);

extern StaticFuncThunk<bool> ft_UTIL_IsCommandIssuedByServerAdmin;
DECL_FT_WRAPPER(UTIL_IsCommandIssuedByServerAdmin);


#if 0
extern StaticFuncThunk<const char *, const char *, int> TranslateWeaponEntForClass;
//const char *TranslateWeaponEntForClass(const char *name, int classnum);
#endif


// TODO: move to common.h
#include <gamestringpool.h>


class CMapListManager
{
public:
	void RefreshList()                      {        ft_RefreshList(this); }
	int GetMapCount() const                 { return ft_GetMapCount(this); }
	int IsMapValid(int index) const         { return ft_IsMapValid (this, index); }
	const char *GetMapName(int index) const { return ft_GetMapName (this, index); }
	
private:
	static MemberFuncThunk<      CMapListManager *, void>              ft_RefreshList;
	static MemberFuncThunk<const CMapListManager *, int>               ft_GetMapCount;
	static MemberFuncThunk<const CMapListManager *, int, int>          ft_IsMapValid;
	static MemberFuncThunk<const CMapListManager *, const char *, int> ft_GetMapName;
};

extern GlobalThunk<CMapListManager> g_MapListMgr;

void PrecacheParticleSystem(const char *name);

void UTIL_HudMessage(CBasePlayer *player, hudtextparms_t & params, const char *message);

void PrintToChatAll(const char *str);

void PrintToChat(const char *str, CTFPlayer *player);

class CEventQueue {
public:
	void AddEvent( const char *target, const char *targetInput, variant_t Value, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID ) {ft_AddEvent(this,target,targetInput,Value,fireDelay,pActivator,pCaller,outputID);}
private:
	static MemberFuncThunk< CEventQueue*, void, const char*,const char *, variant_t, float, CBaseEntity *, CBaseEntity *, int>   ft_AddEvent;
};

extern GlobalThunk<CEventQueue> g_EventQueue;
#endif