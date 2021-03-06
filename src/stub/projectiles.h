#ifndef _INCLUDE_SIGSEGV_STUB_PROJECTILES_H_
#define _INCLUDE_SIGSEGV_STUB_PROJECTILES_H_


#include "link/link.h"
#include "stub/baseanimating.h"


class CBaseProjectile : public CBaseAnimating
{
public:
	CBaseEntity *GetOriginalLauncher() const { return this->m_hOriginalLauncher; }

	int GetProjectileType() const            { return vt_GetProjectileType(this); }
	bool IsDestroyable() const            { return vt_IsDestroyable(this); }
	void Destroy(bool blinkOut = true, bool breakRocket = false) const            { return vt_Destroy(this, blinkOut, breakRocket); }
	void SetLauncher(CBaseEntity *launcher)  { vt_SetLauncher(this, launcher); }
	
private:
	DECL_SENDPROP(CHandle<CBaseEntity>, m_hOriginalLauncher);
	
	static MemberVFuncThunk<const CBaseProjectile *, int> vt_GetProjectileType;
	static MemberVFuncThunk<const CBaseProjectile *, bool> vt_IsDestroyable;
	static MemberVFuncThunk<const CBaseProjectile *, void, bool, bool> vt_Destroy;
	static MemberVFuncThunk<CBaseProjectile *, void, CBaseEntity*> vt_SetLauncher;
};

class CBaseGrenade : public CBaseProjectile {};

class CThrownGrenade : public CBaseGrenade {};
class CBaseGrenadeConcussion : public CBaseGrenade {};
class CBaseGrenadeContact : public CBaseGrenade {};
class CBaseGrenadeTimed : public CBaseGrenade {};

class CTFBaseProjectile : public CBaseProjectile {
public:
	void SetDamage(float damage) { vt_SetDamage(this, damage); }
private:
	static MemberVFuncThunk<CTFBaseProjectile *, void, float> vt_SetDamage;
};

class CTFBaseRocket  : public CBaseProjectile
{
public:
	CBaseEntity *GetLauncher() const { return this->m_hLauncher; }
	
	CBasePlayer *GetOwnerPlayer() const { return ft_GetOwnerPlayer(this); }
	
	DECL_SENDPROP(Vector, m_vInitialVelocity);
	DECL_SENDPROP(int,    m_iDeflected);
	
private:
	DECL_SENDPROP(CHandle<CBaseEntity>, m_hLauncher);
	
	static MemberFuncThunk<const CTFBaseRocket *, CBasePlayer *> ft_GetOwnerPlayer;
};

class CTFFlameRocket : public CTFBaseRocket {};

class CTFProjectile_Syringe : public CTFBaseProjectile {};

class CTFProjectile_EnergyRing : public CTFBaseProjectile
{
public:
	float GetInitialVelocity()  { return ft_GetInitialVelocity(this); }

private:
	static MemberFuncThunk<CTFProjectile_EnergyRing *, float> ft_GetInitialVelocity;
};

class CTFProjectile_Rocket : public CTFBaseRocket {};
class CTFProjectile_SentryRocket : public CTFProjectile_Rocket {};
class CTFProjectile_Flare : public CTFBaseRocket {};
class CTFProjectile_EnergyBall : public CTFBaseRocket {};

class CTFProjectile_Arrow : public CTFBaseRocket
{
public:

	//bool CanPenetrate() const { return ft_CanPenetrate(this); }
	//void SetPenetrate(bool penetrate) { ft_SetPenetrate(this, penetrate); }

	DECL_EXTRACT(float, m_flTimeInit);
	DECL_SENDPROP(bool, m_bCritical);

	static CTFProjectile_Arrow *Create(const Vector &vecOrigin, const QAngle &vecAngles, const float fSpeed, const float fGravity, int projectileType, CBaseEntity *pOwner, CBaseEntity *pScorer) {return ft_Create(vecOrigin, vecAngles, fSpeed, fGravity, projectileType, pOwner, pScorer);}
private:
	static StaticFuncThunk<CTFProjectile_Arrow *,const Vector &, const QAngle &, const float, const float, int, CBaseEntity *, CBaseEntity *> ft_Create;
	//static MemberFuncThunk<const CTFProjectile_Arrow *, bool> ft_CanPenetrate;
	//static MemberFuncThunk<CTFProjectile_Arrow *, void, bool> ft_SetPenetrate;
};

class CTFProjectile_HealingBolt : public CTFProjectile_Arrow {};
class CTFProjectile_GrapplingHook : public CTFProjectile_Arrow {};

class CTFWeaponBaseGrenadeProj : public CBaseGrenade
{
public:
	int GetWeaponID() const { return vt_GetWeaponID(this); }
	
	DECL_SENDPROP(int,    m_iDeflected);
	DECL_SENDPROP(bool,   m_bCritical);
	DECL_SENDPROP(Vector, m_vInitialVelocity);
	
private:
	static MemberVFuncThunk<const CTFWeaponBaseGrenadeProj *, int> vt_GetWeaponID;
};

class CTFGrenadePipebombProjectile : public CTFWeaponBaseGrenadeProj
{
public:
	CBaseEntity *GetLauncher() const { return this->m_hLauncher; }
	
private:
	DECL_SENDPROP(CHandle<CBaseEntity>, m_hLauncher);
};

class CTFWeaponBaseMerasmusGrenade : public CTFWeaponBaseGrenadeProj {};

class CTFProjectile_Jar : public CTFGrenadePipebombProjectile {};
class CTFProjectile_JarMilk : public CTFProjectile_Jar {};
class CTFProjectile_Cleaver : public CTFProjectile_Jar {};

class CTFProjectile_Throwable : public CTFProjectile_Jar {};
class CTFProjectile_ThrowableBreadMonster : public CTFProjectile_Throwable {};
class CTFProjectile_ThrowableBrick : public CTFProjectile_Throwable {};
class CTFProjectile_ThrowableRepel : public CTFProjectile_Throwable {};

class CTFStunBall : public CTFGrenadePipebombProjectile {};
class CTFBall_Ornament : public CTFStunBall {};

class CTFProjectile_SpellFireball : public CTFProjectile_Rocket {};
class CTFProjectile_SpellBats : public CTFProjectile_Jar {};
class CTFProjectile_SpellMirv : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellMeteorShower : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellPumpkin : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellTransposeTeleport : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellSpawnBoss : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellSpawnZombie : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellSpawnHorde : public CTFProjectile_SpellBats {};
class CTFProjectile_SpellLightningOrb : public CTFProjectile_SpellFireball {};
class CTFProjectile_SpellKartOrb : public CTFProjectile_SpellFireball {};
class CTFProjectile_SpellKartBats : public CTFProjectile_SpellBats {};


class IBaseProjectileAutoList
{
public:
	static const CUtlVector<IBaseProjectileAutoList *>& AutoList() { return m_IBaseProjectileAutoListAutoList; }
private:
	static GlobalThunk<CUtlVector<IBaseProjectileAutoList *>> m_IBaseProjectileAutoListAutoList;
};


#endif
