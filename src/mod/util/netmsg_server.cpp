#include "mod.h"
#include "mod/util/netmsg_shared.h"


namespace Mod::Util::NetMsg_Server
{
	
	
	
	class CMod : public IMod
	{
	public:
		CMod() : IMod("Util:NetMsg_Server")
		{
			
		}
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_util_netmsg_server", "0", FCVAR_NOTIFY,
		"Utility: custom net messages: server",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}
