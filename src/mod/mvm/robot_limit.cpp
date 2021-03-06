#include "mod.h"
#include "stub/tfbot.h"
#include "stub/populators.h"
#include "stub/gamerules.h"
#include "util/scope.h"


namespace Mod::MvM::Robot_Limit
{
	void CheckForMaxInvadersAndKickExtras(CUtlVector<CTFPlayer *>& mvm_bots);
	int CollectMvMBots(CUtlVector<CTFPlayer *> *mvm_bots, bool collect_red);

	ConVar cvar_fix_red("sig_mvm_robot_limit_fix_red", "0", FCVAR_NOTIFY,
		"Mod: fix problems with enforcement of the MvM robot limit when bots are on red team");

	ConVar cvar_override("sig_mvm_robot_limit_override", "22", FCVAR_NOTIFY,
		"Mod: override the max number of MvM robots that are allowed to be spawned at once (normally 22)",
		true, 0, false, 0,
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			/* ensure that slots are cleared up when this convar is decreased */
			CUtlVector<CTFPlayer *> mvm_bots;
			CollectMvMBots(&mvm_bots, cvar_fix_red.GetBool());
			CheckForMaxInvadersAndKickExtras(mvm_bots);
			DevMsg("Changed\n");
		});
	
	
	
	int GetMvMInvaderLimit() { return cvar_override.GetInt(); }
	
	
	// reimplement the MvM bots-over-quota logic, with some changes:
	// - use the overridden max bot count, rather than a hardcoded 22
	// - add a third pass to the collect-bots-to-kick logic for bots on TF_TEAM_RED
	void CheckForMaxInvadersAndKickExtras(CUtlVector<CTFPlayer *>& mvm_bots)
	{
		if (mvm_bots.Count() < GetMvMInvaderLimit()) return;
		
		if (mvm_bots.Count() > GetMvMInvaderLimit()) {
			CUtlVector<CTFPlayer *> bots_to_kick;
			int need_to_kick = (mvm_bots.Count() - GetMvMInvaderLimit());
			DevMsg("Need to kick %d bots\n", need_to_kick);
			
			/* pass 1: nominate bots on TEAM_SPECTATOR to be kicked */
			for (auto bot : mvm_bots) {
				if (need_to_kick <= 0) break;
				
				if (bot->GetTeamNumber() == TEAM_SPECTATOR) {
					bots_to_kick.AddToTail(bot);
					--need_to_kick;
				}
			}
			
			/* pass 2: nominate bots on TF_TEAM_RED to be kicked */
			if (cvar_fix_red.GetBool()) {
				for (auto bot : mvm_bots) {
					if (need_to_kick <= 0) break;
					
					if (bot->GetTeamNumber() == TF_TEAM_RED) {
						bots_to_kick.AddToTail(bot);
						--need_to_kick;
					}
				}
			}
			/* pass 3: nominate dead bots to be kicked */
			for (auto bot : mvm_bots) {
				if (need_to_kick <= 0) break;
				
				if (!bot->IsAlive()) {
					bots_to_kick.AddToTail(bot);
					--need_to_kick;
				}
			}
			/* pass 4: nominate bots on TF_TEAM_BLUE to be kicked */
			for (auto bot : mvm_bots) {
				if (need_to_kick <= 0) break;
				
				if (bot->GetTeamNumber() == TF_TEAM_BLUE) {
					bots_to_kick.AddToTail(bot);
					--need_to_kick;
				}
			}

			/* now, kick the bots we nominated */
			for (auto bot : bots_to_kick) {
				engine->ServerCommand(CFmtStr("kickid %d\n", bot->GetUserID()));
			}
		}
	}
	
	
	// rewrite this function entirely, with some changes:
	// - rather than including any CTFPlayer that IsBot, only count TFBots
	// - don't exclude bots who are on TF_TEAM_RED if they have TF_COND_REPROGRAMMED
	// ALSO, do a hacky thing at the end so we can redo the MvM bots-over-quota logic ourselves

	// Returns red bots collected
	int CollectMvMBots(CUtlVector<CTFPlayer *> *mvm_bots, bool collect_red) {
		mvm_bots->RemoveAll();
		int count_red = 0;
		for (int i = 1; i <= gpGlobals->maxClients; ++i) {
			CTFBot *bot = ToTFBot(UTIL_PlayerByIndex(i));
			if (bot == nullptr)      continue;
			if (ENTINDEX(bot) == 0)  continue;
			if (!bot->IsBot())       continue;
			if (!bot->IsConnected()) continue;
			
			if (bot->GetTeamNumber() == TF_TEAM_RED) {
				if (collect_red && bot->m_Shared->InCond(TF_COND_REPROGRAMMED)) {
					count_red += 1;
				} else {
					/* exclude */
					continue;
				}
			}

			mvm_bots->AddToTail(bot);
		}
		return count_red;
	}

	RefCount rc_CPopulationManager_CollectMvMBots;
	DETOUR_DECL_STATIC(int, CPopulationManager_CollectMvMBots, CUtlVector<CTFPlayer *> *mvm_bots)
	{
		CollectMvMBots(mvm_bots, cvar_fix_red.GetBool());
		
		if (rc_CPopulationManager_CollectMvMBots == 0) {
			/* do the bots-over-quota logic ourselves */
			CheckForMaxInvadersAndKickExtras(*mvm_bots);
			
			/* ensure that the original code won't do its own bots-over-quota logic */
			mvm_bots->RemoveAll();
		}
		DevMsg("Collect MVM resulted in %d bots\n", mvm_bots->Count());
		return mvm_bots->Count();
	}
	
	// rewrite this function entirely, with some changes:
	// - use the overridden max bot count, rather than a hardcoded 22
	bool allocate_round_start = false;
	bool hibernated = false;

	DETOUR_DECL_MEMBER(void, CPopulationManager_AllocateBots)
	{
		
		auto popmgr = reinterpret_cast<CPopulationManager *>(this);

		SCOPED_INCREMENT(rc_CPopulationManager_CollectMvMBots);
		
		if (!allocate_round_start) return;
		
		CUtlVector<CTFPlayer *> mvm_bots;
		int num_bots = CPopulationManager::CollectMvMBots(&mvm_bots);
		
		if (num_bots > 0 && !allocate_round_start) {
			Warning("%d bots were already allocated some how before CPopulationManager::AllocateBots was called\n", num_bots);
		}
		
		CheckForMaxInvadersAndKickExtras(mvm_bots);
		
		while (num_bots < GetMvMInvaderLimit()) {
			CTFBot *bot = NextBotCreatePlayerBot<CTFBot>("TFBot", false);
			if (bot != nullptr) {
				bot->ChangeTeam(TEAM_SPECTATOR, false, true, false);
			}
			
			++num_bots;
		}
		
		popmgr->m_bAllocatedBots = true;
	}
	//Restore the original max bot counts
	RefCount rc_JumpToWave;

	THINK_FUNC_DECL(SpawnBots)
	{
		allocate_round_start = true;
		g_pPopulationManager->AllocateBots();
		allocate_round_start = false;
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_JumpToWave, unsigned int wave, float f1)
	{
		SCOPED_INCREMENT(rc_JumpToWave);
		//DevMsg("[%8.3f] JumpToWaveRobotlimit\n", gpGlobals->curtime);
		DETOUR_MEMBER_CALL(CPopulationManager_JumpToWave)(wave, f1);
		

		THINK_FUNC_SET(g_pPopulationManager, SpawnBots, gpGlobals->curtime + 0.12f);

	}
	
	DETOUR_DECL_MEMBER(void, CPopulationManager_WaveEnd, bool b1)
	{
		//DevMsg("[%8.3f] WaveEnd\n", gpGlobals->curtime);
		DETOUR_MEMBER_CALL(CPopulationManager_WaveEnd)(b1);

		CUtlVector<CTFPlayer *> mvm_bots;
		CollectMvMBots(&mvm_bots, cvar_fix_red.GetBool());
		CheckForMaxInvadersAndKickExtras(mvm_bots);
	}
	
	DETOUR_DECL_MEMBER(void, CMannVsMachineStats_RoundEvent_WaveEnd, bool success)
	{
		DETOUR_MEMBER_CALL(CMannVsMachineStats_RoundEvent_WaveEnd)(success);
		if (!success && rc_JumpToWave == 0) {
			//DevMsg("[%8.3f] RoundEvent_WaveEnd\n", gpGlobals->curtime);
			CUtlVector<CTFPlayer *> mvm_bots;
			CollectMvMBots(&mvm_bots, cvar_fix_red.GetBool());
			CheckForMaxInvadersAndKickExtras(mvm_bots);
		}
	}

	DETOUR_DECL_MEMBER(ActionResult< CTFBot >, CTFBotDead_Update, CTFBot *bot, float interval)
	{
		auto result = DETOUR_MEMBER_CALL(CTFBotDead_Update)(bot,interval);
		if (result.transition == ActionTransition::DONE) {
			CUtlVector<CTFPlayer *> mvm_bots;
			CollectMvMBots(&mvm_bots, true);
			if (mvm_bots.Count() > GetMvMInvaderLimit()) {
				engine->ServerCommand(CFmtStr("kickid %d\n", bot->GetUserID()));
			}
		}
		return result;
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_StartCurrentWave)
	{
		DETOUR_MEMBER_CALL(CPopulationManager_StartCurrentWave)();
	}

	DETOUR_DECL_MEMBER(void, CGameServer_SetHibernating, bool hibernate)
	{
		if (!hibernate && hibernated)
		{
			THINK_FUNC_SET(g_pPopulationManager, SpawnBots, gpGlobals->curtime + 0.12f);
		}
		hibernated = hibernate;
		DETOUR_MEMBER_CALL(CGameServer_SetHibernating)(hibernate);
	}

	class CMod : public IMod
	{
	public:
		CMod() : IMod("MvM:Robot_Limit")
		{
			//MOD_ADD_DETOUR_MEMBER(CTFBotSpawner_Spawn,               "CTFBotSpawner::Spawn");
			MOD_ADD_DETOUR_STATIC(CPopulationManager_CollectMvMBots, "CPopulationManager::CollectMvMBots");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_AllocateBots,   "CPopulationManager::AllocateBots");

			MOD_ADD_DETOUR_MEMBER(CPopulationManager_JumpToWave,          "CPopulationManager::JumpToWave");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_WaveEnd,             "CPopulationManager::WaveEnd");
			MOD_ADD_DETOUR_MEMBER(CMannVsMachineStats_RoundEvent_WaveEnd, "CMannVsMachineStats::RoundEvent_WaveEnd");

			MOD_ADD_DETOUR_MEMBER(CTFBotDead_Update, "CTFBotDead::Update");

			MOD_ADD_DETOUR_MEMBER(CGameServer_SetHibernating, "CGameServer::SetHibernating");
		}
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_mvm_robot_limit", "0", FCVAR_NOTIFY,
		"Mod: modify/enhance/fix some population manager code related to the 22-robot limit in MvM",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}
