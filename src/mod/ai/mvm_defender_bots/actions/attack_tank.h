#ifndef _INCLUDE_SIGSEGV_MOD_AI_MVM_DEFENDER_BOTS_ACTIONS_ATTACK_TANK_H_
#define _INCLUDE_SIGSEGV_MOD_AI_MVM_DEFENDER_BOTS_ACTIONS_ATTACK_TANK_H_


#include "re/nextbot.h"
#include "re/path.h"
#include "stub/tfbot.h"
#include "stub/entities.h"


namespace Mod::AI::MvM_Defender_Bots
{
	/* attack a tank */
	class CTFBotAttackTank : public IHotplugAction
	{
	public:
		CTFBotAttackTank() {}
		
		virtual const char *GetName() const override { return "AttackTank"; }
		
		virtual QueryResponse ShouldHurry(const INextBot *nextbot) const override   { return QueryResponse::YES; }
		virtual QueryResponse ShouldRetreat(const INextBot *nextbot) const override { return QueryResponse::NO; }
		
		virtual ActionResult<CTFBot> OnStart(CTFBot *actor, Action<CTFBot> *action) override;
		virtual ActionResult<CTFBot> Update(CTFBot *actor, float dt) override;
		virtual void OnEnd(CTFBot *actor, Action<CTFBot> *action) override;
		
		virtual EventDesiredResult<CTFBot> OnMoveToSuccess(CTFBot *actor, const Path *path) override;
		virtual EventDesiredResult<CTFBot> OnMoveToFailure(CTFBot *actor, const Path *path, MoveToFailureType fail) override;
		virtual EventDesiredResult<CTFBot> OnStuck(CTFBot *actor) override;
		
		static bool IsPossible(CTFBot *actor);
		
	private:
		bool SelectTarget();
		
		void DrawOverlays();
		
		float GetIdealTankRange();
		
		void EquipBestTankWeapon();
		
		CHandle<CTFTankBoss> m_hTarget;
		
		PathFollower m_PathFollower;
		CountdownTimer m_ctRecomputePath;
	};
}


#endif
