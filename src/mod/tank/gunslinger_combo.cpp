#include "mod.h"
#include "stub/tfweaponbase.h"
#include "stub/tf_shareddefs.h"
#include "util/iterate.h"


namespace Mod::Tank::Gunslinger_Combo
{
	


	RefCount rc_CTFRobotArm_Smack;
	DETOUR_DECL_MEMBER(void, CTFRobotArm_Smack)
	{
		SCOPED_INCREMENT(rc_CTFRobotArm_Smack);
		DETOUR_MEMBER_CALL(CTFRobotArm_Smack)();
	}

	DETOUR_DECL_MEMBER(bool, CTFWeaponBaseMelee_DoSwingTrace, trace_t &trace)
	{
		bool result = DETOUR_MEMBER_CALL(CTFWeaponBaseMelee_DoSwingTrace)(trace);
		if (rc_CTFRobotArm_Smack && result && trace.m_pEnt != nullptr && ENTINDEX(trace.m_pEnt) != 0 && reinterpret_cast<CBaseEntity *>(trace.m_pEnt)->ClassMatches("tank_boss")) {
			--rc_CTFRobotArm_Smack;
			CBaseEntity *owner = reinterpret_cast<CTFWeaponBaseMelee *>(this)->GetOwnerEntity();

			ForEachPlayer([&](CBasePlayer *player){
				DevMsg("has\n");
				if (player->GetTeamNumber() != owner->GetTeamNumber()) {
					trace.m_pEnt = player;
					return false;
				}
				return true;
			});
		}
		return result;
	}

	class CMod : public IMod
	{
	public:
		CMod() : IMod("Tank:Gunslinger_Combo") {
			

			MOD_ADD_DETOUR_MEMBER(CTFRobotArm_Smack, "CTFRobotArm::Smack");
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBaseMelee_DoSwingTrace, "CTFWeaponBaseMelee::DoSwingTrace");
		}
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_tank_gunslinger_combo", "0", FCVAR_NOTIFY,
		"Tank: enable gunslinger 3-punch combo functionality",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}
