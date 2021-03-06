/* reverse engineering by sigsegv
 * based on TF2 version 20151007a
 * server/tf/bot/behavior/tf_bot_dead.h
 * used in MvM: TODO
 */


// sizeof: 0x38
class CTFBotDead : public Action<CTFBot>
{
public:
	CTFBotDead();
	virtual ~CTFBotDead();
	
	virtual const char *GetName() const override;
	
	virtual ActionResult<CTFBot> OnStart(CTFBot *actor, Action<CTFBot> *action) override;
	virtual ActionResult<CTFBot> Update(CTFBot *actor, float dt) override;
	
private:
	IntervalTimer m_itDeathEpoch; // +0x34
};
