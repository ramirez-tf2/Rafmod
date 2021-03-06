#ifndef _INCLUDE_SIGSEGV_STUB_TEAM_H_
#define _INCLUDE_SIGSEGV_STUB_TEAM_H_


#include "stub/baseentity.h"
#include "prop.h"
#include "link/link.h"


class CBasePlayer;
class CTFPlayer;
class CTeamSpawnPoint;
class CBaseObject;


class CTeam : public CBaseEntity
{
public:
	int GetTeamNumber() const          { return vt_GetTeamNumber(this); }
	const char *GetName()              { return vt_GetName      (this); }
	int GetNumPlayers()                { return vt_GetNumPlayers(this); }
	CBasePlayer *GetPlayer(int iIndex) { return vt_GetPlayer    (this, iIndex); }
	
private:
	static MemberVFuncThunk<const CTeam *, int>                vt_GetTeamNumber;
	static MemberVFuncThunk<      CTeam *, const char *>       vt_GetName;
	static MemberVFuncThunk<      CTeam *, int>                vt_GetNumPlayers;
	static MemberVFuncThunk<      CTeam *, CBasePlayer *, int> vt_GetPlayer;
};


// g_Teams
// GetGlobalTeam
// GetNumberOfTeams


class CTFTeam : public CTeam
{
public:
	int GetNumObjects(int iObjectType = -1) { return ft_GetNumObjects(this, iObjectType); }
	CBaseObject *GetObject(int num)         { return ft_GetObject    (this, num); }
	
private:
	static MemberFuncThunk<CTFTeam *, int, int>           ft_GetNumObjects;
	static MemberFuncThunk<CTFTeam *, CBaseObject *, int> ft_GetObject;
};


// GetGlobalTFTeam


class CTFTeamManager
{
public:
	bool IsValidTeam(int iTeam) { return ft_IsValidTeam(this, iTeam); }
	CTFTeam *GetTeam(int iTeam) { return ft_GetTeam    (this, iTeam); }
	
private:
	static MemberFuncThunk<CTFTeamManager *, bool, int>      ft_IsValidTeam;
	static MemberFuncThunk<CTFTeamManager *, CTFTeam *, int> ft_GetTeam;
};


extern GlobalThunk<CTFTeamManager> s_TFTeamManager;
inline CTFTeamManager *TFTeamMgr() { return &(s_TFTeamManager.GetRef()); }


#endif
