#ifndef _INCLUDE_SIGSEGV_UTIL_ADMIN_H_
#define _INCLUDE_SIGSEGV_UTIL_ADMIN_H_


constexpr FlagBits ADMFLAG_NONE = (FlagBits)0;
constexpr FlagBits ADMFLAG_ALL = ((FlagBits)(1 << AdminFlags_TOTAL) - 1);
static_assert(ADMFLAG_ALL == 0x001fffff); // this check is based on the SM admin flags as of 20180405


AdminId GetPlayerSMAdminID(CBasePlayer *player);

int GetSMTargets(CBasePlayer *caller, const char *pattern, std::vector<CBasePlayer *> &vec, char *target_name = nullptr, int target_name_size = 0, int flags = 0);

bool PlayerIsSMAdmin     (CBasePlayer *player);
bool PlayerIsSMAdminOrBot(CBasePlayer *player);

FlagBits GetPlayerSMAdminFlags(CBasePlayer *player);

bool PlayerHasSMAdminFlag(CBasePlayer *player, AdminFlag flag);

bool PlayerHasSMAdminFlags_All(CBasePlayer *player, FlagBits flag_mask);
bool PlayerHasSMAdminFlags_Any(CBasePlayer *player, FlagBits flag_mask);


#endif
