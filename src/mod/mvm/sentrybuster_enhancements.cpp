#include "mod.h"
#include "re/nextbot.h"
#include "stub/tfbot.h"

#include <effect_dispatch_data.h>


namespace Mod::MvM::SentryBuster_Enhancements
{
	class SendProxyOverride
	{
	public:
		SendProxyOverride(const char *obj, const char *mem) :
			m_strObj(obj), m_strMem(mem) {}
		~SendProxyOverride()
		{
			this->Restore();
		}
		
		bool Override(SendVarProxyFn proxy)
		{
			if (this->m_bActive) {
				Warning("SendProxyOverride: %s::%s is already overridden\n", this->m_strObj.c_str(), this->m_strMem.c_str());
				return false;
			}
			
			sm_sendprop_info_t info;
			if (!gamehelpers->FindSendPropInfo(this->m_strObj.c_str(), this->m_strMem.c_str(), &info)) {
				Warning("SendProxyOverride: couldn't find info for %s::%s\n", this->m_strObj.c_str(), this->m_strMem.c_str());
				return false;
			}
			
			this->m_pProp = info.prop;
			this->m_pSavedProxy = info.prop->GetProxyFn();
			
			info.prop->SetProxyFn(proxy);
			
			this->m_bActive = true;
			
			return true;
		}
		
		void Restore()
		{
			if (this->m_bActive) {
				this->m_pProp->SetProxyFn(this->m_pSavedProxy);
				
				this->m_bActive = false;
			}
		}
		
	private:
		std::string m_strObj;
		std::string m_strMem;
		
		bool m_bActive = false;
		SendProp *m_pProp = nullptr;
		SendVarProxyFn m_pSavedProxy = nullptr;
	};
	
	
	// try: DispatchParticleEffect
	// try: glow
	// try: create entity and do FollowEntity
	// try: create entity and do SetParent
	
	
	void DispatchParticleEffect_Filtered(const char *pszParticleName, ParticleAttachment_t iAttachType, CBaseEntity *pEntity, const char *pszAttachmentName, bool bResetAllParticles = false)
	{
		int iAttachment = -1;
		
#if 0
		auto pAnimating = rtti_cast<CBaseAnimating *>(pEntity);
		if (pAnimating != nullptr) {
			iAttachment = pAnimating->LookupAttachment(pszAttachmentName);
			if (iAttachment <= 0) {
				Warning("Model '%s' doesn't have attachment '%s' to attach particle system '%s' to.\n", STRING(pAnimating->GetModelName()), pszAttachmentName, pszParticleName);
				return;
			}
		}
#endif
		
		// TODO
		
		
	}
	
	
	/* works */
	CON_COMMAND(sig_mvm_sentrybuster_enhancements_test1, "DispatchParticleEffect")
	{
		INextBot *selected = TheNextBots().GetSelectedBot();
		if (selected == nullptr) {
			DevMsg("No bot selected.\n");
			return;
		}
		
		CTFBot *bot = ToTFBot(selected->GetEntity());
		if (bot == nullptr) {
			DevMsg("Invalid bot.\n");
			return;
		}
		
		if (args.ArgC() < 3) {
			DevMsg("Not enough arguments.\n");
			return;
		}
		
		const char *attach = args[1];
		const char *effect = args[2];
		
		DispatchParticleEffect(effect, PATTACH_POINT_FOLLOW, bot, attach);
	}
	
	
	/* works */
	CON_COMMAND(sig_mvm_sentrybuster_enhancements_test2, "GlowEffect")
	{
		INextBot *selected = TheNextBots().GetSelectedBot();
		if (selected == nullptr) {
			DevMsg("No bot selected.\n");
			return;
		}
		
		CTFBot *bot = ToTFBot(selected->GetEntity());
		if (bot == nullptr) {
			DevMsg("Invalid bot.\n");
			return;
		}
		
		if (args.ArgC() < 2) {
			DevMsg("Not enough arguments.\n");
			return;
		}
		
		const char *onoff = args[1];
		
		if (V_stricmp(onoff, "on") == 0) {
			bot->AddGlowEffect();
		} else if (V_stricmp(onoff, "off") == 0) {
			bot->RemoveGlowEffect();
		} else {
			DevMsg("Bad argument.\n");
		}
	}
	
	
	void GlowProxy(const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID)
	{
		DevMsg("tick %d, objectID %d\n", gpGlobals->tickcount, objectID);
		pOut->m_Int = (RandomFloat(0.0f, 1.0f) > 0.50f ? 1 : 0);
	}
	
	
	class CMod : public IMod
	{
	public:
		CMod() : IMod("MvM:SentryBuster_Enhancements"),
			m_GlowProxy("CBaseCombatCharacter", "m_bGlowEnabled")
		{
			
		}
		
		virtual void OnEnable() override
		{
			DevMsg("enable\n");
			this->m_GlowProxy.Override(GlowProxy);
		}
		virtual void OnDisable() override
		{
			DevMsg("disable\n");
			this->m_GlowProxy.Restore();
		}
		
	private:
		SendProxyOverride m_GlowProxy;
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_mvm_sentrybuster_enhancements", "0", FCVAR_NOTIFY,
		"Mod: enhanced sentry busters",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}
