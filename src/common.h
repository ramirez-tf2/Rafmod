/* precompiled header! */

#ifndef _INCLUDE_SIGSEGV_COMMON_H_
#define _INCLUDE_SIGSEGV_COMMON_H_


#if defined __cplusplus


#if defined __GNUC__ || defined __clang__
	#define PRAGMA(str) _Pragma(#str)
#elif defined _MSC_VER
	#define PRAGMA(str) __pragma(str)
#endif

#if defined __GNUC__
	#define WARN_RESTORE() PRAGMA(GCC diagnostic pop)
#elif defined __clang__
	#define WARN_RESTORE() PRAGMA(clang diagnostic pop)
#else
	#define WARN_RESTORE()
#endif

#if defined __GNUC__
	#define WARN_IGNORE(x) PRAGMA(GCC diagnostic push) PRAGMA(GCC diagnostic ignored x)
#elif defined __clang__
	#define WARN_IGNORE(x) PRAGMA(clang diagnostic push) PRAGMA(clang diagnostic ignored x)
#else
	#define WARN_IGNORE(x)
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__SIGN_COMPARE() WARN_IGNORE("-Wsign-compare")
#else
	#define WARN_IGNORE__SIGN_COMPARE()
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__ADDRESS() WARN_IGNORE("-Waddress")
#else
	#define WARN_IGNORE__ADDRESS()
#endif

#if defined __GNUC__
	#define WARN_IGNORE__NONNULL_COMPARE() WARN_IGNORE("-Wnonnull-compare")
#else
	#define WARN_IGNORE__NONNULL_COMPARE()
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__REORDER() WARN_IGNORE("-Wreorder")
#else
	#define WARN_IGNORE__REORDER()
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__CONVERSION_NULL() WARN_IGNORE("-Wconversion-null")
#else
	#define WARN_IGNORE__CONVERSION_NULL()
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__NULL_DEREFERENCE() WARN_IGNORE("-Wnull-dereference")
#else
	#define WARN_IGNORE__NULL_DEREFERENCE()
#endif

#if defined __GNUC__ || defined __clang__
	#define WARN_IGNORE__CLASS_MEMACCESS() WARN_IGNORE("-Wclass-memaccess")
#else
	#define WARN_IGNORE__CLASS_MEMACCESS()
#endif


class IVEngineServer;
class IServerGameDLL;
class IFileSystem;
class IServerGameClients;
class IServer;
class ICvar;
class ISpatialPartition;
class IEngineTrace;
class IStaticPropMgrServer;
class IGameEventManager2;
class INetworkStringTableContainer;
class IEngineSound;
class IVModelInfo;
class IVDebugOverlay;

class IPlayerInfoManager;
class IBotManager;

class IPhysics;
class IPhysicsCollision;

class ISoundEmitterSystemBase;

class IMaterialSystem;

namespace vgui {
	class IVGui;
	class IInput;
	class IPanel;
	class ISchemeManager;
	class ISystem;
	class ILocalize;
	class IInputInternal;
	
	class ISurface;
}
class IMatSystemSurface;

class CGlobalVars;
class CGlobalEntityList;
class CBaseEntityList;

class IVEngineClient;
class IBaseClientDLL;
class IClientEntityList;

class IEngineTool;
class IServerTools;
class IClientTools;

class IVProfExport;

class IDedicatedExports;

class IMDLCache;

namespace SourceMod {
	class IExtensionManager;
}

class IClientMode;


extern IVEngineServer *engine;
extern IServerGameDLL *gamedll;
extern IFileSystem *filesystem;
extern IServerGameClients *serverGameClients;
extern IServer *sv;
extern ICvar *icvar;
extern ISpatialPartition *partition;
extern IEngineTrace *enginetrace;
extern IStaticPropMgrServer *staticpropmgr;
extern IGameEventManager2 *gameeventmanager;
extern INetworkStringTableContainer *networkstringtable;
extern IEngineSound *enginesound;
extern IVModelInfo *modelinfo;
extern IVDebugOverlay *debugoverlay;

extern IPlayerInfoManager *playerinfomanager;
extern IBotManager *botmanager;

extern IPhysics *physics;
extern IPhysicsCollision *physcollision;

extern ISoundEmitterSystemBase *soundemitterbase;

extern IMaterialSystem *g_pMaterialSystem;

extern vgui::IVGui *g_pVGui;
extern vgui::IInput *g_pVGuiInput;
extern vgui::IPanel *g_pVGuiPanel;
extern vgui::ISchemeManager *g_pVGuiSchemeManager;
extern vgui::ISystem *g_pVGuiSystem;
extern vgui::ILocalize *g_pVGuiLocalize;
extern vgui::IInputInternal *g_pVGuiInputInternal;

extern vgui::ISurface *g_pVGuiSurface;
extern IMatSystemSurface *g_pMatSystemSurface;

extern CGlobalVars *gpGlobals;
extern CGlobalEntityList *gEntList;
extern CBaseEntityList *g_pEntityList;

extern IVEngineClient *engineclient;
extern IBaseClientDLL *clientdll;
extern IClientEntityList *cl_entitylist;

extern IEngineTool *enginetools;
extern IServerTools *servertools;
extern IClientTools *clienttools;

extern IVProfExport *vprofexport;

extern IDedicatedExports *dedicated;

extern IMDLCache *mdlcache;

extern IClientMode *g_pClientMode;


/* C standard library */
#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <cmath>
#include <cfloat>


/* C++ standard library */
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
//#include <random> // <-- costs  ~89ms per compile
//#include <regex>  // <-- costs ~285ms per compile
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <utility>
using namespace std::literals;


/* STL */
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>


/* Boost */


/* Posix specific */
#if defined _LINUX || defined _OSX

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef DEBUG

#include <libunwind.h>

#endif

#define HAVE_DECL_BASENAME 1
#include <libiberty/demangle.h>
#include <libelf.h>

#endif


/* Windows specific */
#if defined _WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable:4091)
#include <Dbghelp.h>
#pragma warning(default:4091)

#include <Winsock2.h>
#include <WS2tcpip.h>

/* namespace clash between Windows CreateEvent macro and IGameEventManager2::CreateEvent */
#if defined CreateEvent
#undef CreateEvent
#endif

#endif


/* LZ4 */
//#include <lz4.h>


/* LodePNG */
//#include <lodepng.h>


/* Capstone */
//#include <capstone.h>


/* Approximate Nearest Neighbor */
//#include <ANN/ANN.h>


/* Source's public/video/ivideoservices.h is naughty and uses the preprocessor
 * to override nullptr; so prevent that header from being included */
#define IVIDEOSERVICES_H
class IVideoRecorder;


/* Source SDK */
#define RAD_TELEMETRY_DISABLED
#undef   NO_STRING_T
#undef WEAK_STRING_T
#include "sdk2013/platform.h"
#include <interface.h>
#include "sdk2013/commonmacros.h"
#include "sdk2013/basetypes.h"
WARN_IGNORE__ADDRESS()
#include <dbg.h>
WARN_RESTORE()
#undef COMPILE_TIME_ASSERT
#define COMPILE_TIME_ASSERT(pred) static_assert(pred)
#include "sdk2013/Color.h"
#include "sdk2013/vector2d.h"
#include "sdk2013/vector.h"
#include "sdk2013/vector4d.h"
#include "sdk2013/mathlib.h"
#include <string_t.h>
#include <annotations.h>
WARN_IGNORE__CLASS_MEMACCESS()
#include <utlmemory.h>
WARN_RESTORE()
#include <utlstring.h>
#include <utlvector.h>
WARN_IGNORE__REORDER()
#include <utlrbtree.h>
WARN_RESTORE()
#include <utlmap.h>
WARN_IGNORE__SIGN_COMPARE()
#include <utlbuffer.h>
WARN_RESTORE()
#include <shareddefs.h>
#include <icvar.h>
#include "sdk2013/convar.h"
#include <fmtstr.h>
#include <KeyValues.h>
#include <ehandle.h>
#include <datamap.h>
#include <predictioncopy.h>
#include <takedamageinfo.h>
#include <iserverentity.h>
WARN_IGNORE__ADDRESS()
WARN_IGNORE__NONNULL_COMPARE()
#include <edict.h>
WARN_RESTORE()
WARN_RESTORE()
#include <IEngineTrace.h>
#include <IStaticPropMgr.h>
#include <ivdebugoverlay.h>
#include "sdk2013/debugoverlay_shared.h"
#include <util_shared.h>
#include <ai_activity.h>
#include <usercmd.h>
#include <npcevent.h>
#include <dt_send.h>
#include <bitbuf.h>
#include <eiface.h>
#include <ispatialpartition.h>
#include <random.h>
#include <ai_activity.h>
#include <igameevents.h>
#include <inetmessage.h>
#include <inetchannel.h>
#include <irecipientfilter.h>
//#include <usermessages.h>
#include <cdll_int.h>
#include <iclientnetworkable.h>
#include <utldict.h>
#include <mp_shareddefs.h>
WARN_IGNORE__REORDER()
#include <bitmap/imageformat.h>
WARN_RESTORE()
//#include <materialsystem/imaterialsystem.h>
#include "sdk2013/imaterialsystem_V081.h"
#include <materialsystem/imaterial.h>
#include <particle_parse.h>
#include <effect_dispatch_data.h>
#include <SoundEmitterSystem/isoundemittersystembase.h>
#include <IEngineSound.h>
#include <igamesystem.h>
#include <vphysics_interface.h>
WARN_IGNORE__CLASS_MEMACCESS()
#include <studio.h>
WARN_RESTORE()
WARN_IGNORE__REORDER()
#include <istudiorender.h>
WARN_RESTORE()
#include <ivmodelrender.h>
#include <vcollide_parse.h>
#include <steam/steamclientpublic.h>
#include <netadr.h>
WARN_IGNORE__NULL_DEREFERENCE()
#include <iclientrenderable.h>
WARN_RESTORE()
#include <toolframework/ienginetool.h>
#include <toolframework/itoolentity.h>
WARN_IGNORE__REORDER()
#include <vprof.h>
WARN_RESTORE()
#include <networkstringtabledefs.h>
#include <valve_minmax_off.h>
#include <stringpool.h>
#include <filesystem.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <VGuiMatSurface/IMatSystemSurface.h>
#include <IKeyValuesSystem.h>
#include <model_types.h>
#include <idedicatedexports.h>
#include <icommandline.h>
#include <iserver.h>
#include <iclient.h>
#include <datacache/imdlcache.h>
WARN_IGNORE__SIGN_COMPARE()
WARN_IGNORE__REORDER()
#include <materialsystem/imesh.h>
WARN_RESTORE()
WARN_RESTORE()
#include <../server/iplayerinfo.h>
#include <icliententity.h>
#include <client_class.h>
#include <../server/iscorer.h>
#include <../client/iclientmode.h>
WARN_IGNORE__CONVERSION_NULL()
#include <vgui_controls/Frame.h>
WARN_RESTORE()
#include <soundflags.h>
#include <overlaytext.h>
#include <tier2/meshutils.h>
#include <server_class.h>
#include <dt_utlvector_common.h>
WARN_IGNORE__REORDER()
#include <../server/variant_t.h>
WARN_RESTORE()
#include <icliententity.h>
#include <icliententitylist.h>

#define DECLARE_PREDICTABLE()
#include <collisionproperty.h>


// can't include game/shared/particle_property.h because it tries to include "particles_new.h" which is in game/client
// so we'll just put what we need here... sigh
#define INVALID_PARTICLE_ATTACHMENT -1

/* the order in which headers include each other can be tricky; so for the SDK2013 headers we've messed with, we'll
 * always put a special preprocessor definition INSIDE of the already-included check in our modified version; that way,
 * we can verify that OUR version of the header was encountered first and took precedence, not the original one
 * (if our version did not take precedence, then these static_assert's won't even fail per se, they'll just encounter an
 * undefined name that they can't deal with and the compiler will become angry) */
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_TIER0_PLATFORM_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_TIER0_COMMONMACROS_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_TIER0_BASETYPES_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_COLOR_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_MATHLIB_VECTOR2D_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_MATHLIB_VECTOR_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_MATHLIB_VECTOR4D_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_MATHLIB_MATHLIB_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_TIER1_CONVAR_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__GAME_SHARED_DEBUGOVERLAY_SHARED_H);
static_assert(_SIGSEGV_SDK2013_OVERRIDE__PUBLIC_MATERIALSYSTEM_IMATERIALSYSTEM_H);


/* AMTL */


/* MetaMod */


/* SourceMod */
#include "sm/MemoryUtils.h"


/* this extension */
#include <smsdk_ext.h>


/* dammit Valve, why do you have to do stupid shit like redefining offsetof?
 * maybe you live in a world where it's impossible to overload operator&, but
 * some of us use non-ancient development tools where a proper, working offsetof
 * is actually important for things to work right */
#undef offsetof
#define offsetof(OBJECT, MEMBER) reinterpret_cast<size_t>(std::addressof(((OBJECT *)nullptr)->MEMBER))


#endif


#endif
