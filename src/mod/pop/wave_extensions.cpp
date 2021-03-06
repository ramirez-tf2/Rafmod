#include "mod.h"
#include "stub/populators.h"
#include "mod/pop/kv_conditional.h"
#include "stub/usermessages_sv.h"
#include "stub/entities.h"
#include "stub/objects.h"
#include "stub/gamerules.h"
#include "stub/tfplayer.h"
#include "stub/team.h"
#include "util/scope.h"
#include "util/iterate.h"
#include "stub/strings.h"
#include "stub/misc.h"
#include "mod/pop/pointtemplate.h"
#include "mod/pop/common.h"

namespace Mod::Pop::Wave_Extensions
{
	struct SentryGunInfo
	{
		~SentryGunInfo()
		{
			if (sentry != nullptr) {
				sentry->DetonateObject();
			}
		}
		
		bool use_hint = true;
		std::vector<CHandle<CTFBotHintSentrygun>> hints;
		Vector origin;
		QAngle angles;
		
		int teamnum = TF_TEAM_BLUE;
		float delay = 0.0f;
		int level   = 0;
		
		bool is_mini = false;
		int health = -1;
		bool spawned = false;

		int skin = 0;
		int bodygroup = 0;
		
		CHandle<CObjectSentrygun> sentry;
	};
	
	struct BossInfo
	{
		~BossInfo()
		{
			if (boss != nullptr) {
				boss->Remove();
			}
		}
		
		Vector origin;
		
		CHalloweenBaseBoss::HalloweenBossType type = CHalloweenBaseBoss::INVALID;
		int teamnum = TF_TEAM_HALLOWEEN_BOSS;
		int health  = -1;
		float delay = 0.0f;
		
		bool spawned = false;
		CHandle<CHalloweenBaseBoss> boss;
	};
	

	struct WaveData
	{
		std::vector<std::string>   explanation;
		std::vector<SentryGunInfo> sentryguns;
		std::vector<BossInfo>      bosses;
		std::map<std::string,float>   sound_loops;
		std::vector<PointTemplateInfo>   templ;
		std::vector<PointTemplateInstance *>   templ_inst;
		std::vector<ETFCond>  addconds;
		std::vector<ETFCond>  addconds_class[10] = {};
		
		bool red_team_wipe_causes_wave_loss = false;
		bool blue_team_wipe_causes_wave_loss = false;
		bool finishing_wave_causes_wave_loss = false;
		bool finishing_wave_and_player_wipe_causes_wave_loss = false;
		bool defined_class_attributes = false;
		std::map<std::string,float> player_attributes_class[10] = {};
		std::map<std::string,float> player_attributes;
		float sound_time_end = 0.f;
		IntervalTimer t_wavestart;
		IntervalTimer t_waveinit;

		CBaseEntity *explanation_text;
	};
	
	
	std::map<CWave *, WaveData> waves;
	
	void WaveCleanup(CWave *wave){
		for (auto inst : waves[wave].templ_inst) {
			if (inst != nullptr)
				inst->OnKilledParent(false);
		}
		waves[wave].templ_inst.clear();
	}
	DETOUR_DECL_MEMBER(void, CWave_dtor0)
	{
		auto wave = reinterpret_cast<CWave *>(this);
		
//		DevMsg("CWave %08x: dtor0\n", (uintptr_t)wave);
		WaveCleanup(wave);
		waves.erase(wave);
		
		DETOUR_MEMBER_CALL(CWave_dtor0)();
	}
	
	DETOUR_DECL_MEMBER(void, CWave_dtor2)
	{
		auto wave = reinterpret_cast<CWave *>(this);
		WaveCleanup(wave);
//		DevMsg("CWave %08x: dtor2\n", (uintptr_t)wave);
		waves.erase(wave);
		
		DETOUR_MEMBER_CALL(CWave_dtor2)();
	}
	
	
	bool FindSentryHint(const char *name, std::vector<CHandle<CTFBotHintSentrygun>>& hints)
	{
		ForEachEntityByClassname("bot_hint_sentrygun", [&](CBaseEntity *ent){
			if (FStrEq(STRING(ent->GetEntityName()), name)) {
				auto hint = rtti_cast<CTFBotHintSentrygun *>(ent);
				if (hint != nullptr) {
					hints.emplace_back(hint);
				}
			}
		});
		
		return !hints.empty();
	}
	
	
	void Parse_Explanation(CWave *wave, KeyValues *kv)
	{
		waves[wave].explanation.clear();
		
		FOR_EACH_SUBKEY(kv, subkey) {
			waves[wave].explanation.emplace_back(subkey->GetString());
		}
	}
	
	void Parse_SentryGun(CWave *wave, KeyValues *kv)
	{
		SentryGunInfo info;
		
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			if (FStrEq(name, "HintName")) {
				if (!FindSentryHint(subkey->GetString(), info.hints)) {
					Warning("Could not find a bot_hint_sentrygun entity named \"%s\".\n", subkey->GetString());
					return;
				}
			} else if (FStrEq(name, "TeamNum")) {
				info.teamnum = Clamp(subkey->GetInt(), (int)TF_TEAM_RED, (int)TF_TEAM_BLUE);
			//	DevMsg("TeamNum \"%s\" --> %d\n", subkey->GetString(), info.teamnum);
			} else if (FStrEq(name, "Delay")) {
				info.delay = Max(0.0f, subkey->GetFloat());
			//	DevMsg("Delay \"%s\" --> %.1f\n", subkey->GetString(), info.delay);
			} else if (FStrEq(name, "Level")) {
				info.level = Clamp(subkey->GetInt(), 1, 3);
			//	DevMsg("Level \"%s\" --> %d\n", subkey->GetString(), info.level);
			} else if (FStrEq(name, "IsMini")) {
				info.is_mini = subkey->GetBool();
			//	DevMsg("Level \"%s\" --> %d\n", subkey->GetString(), info.level);
			} else if (FStrEq(name, "Health")) {
				info.health = subkey->GetInt();
			//	DevMsg("Level \"%s\" --> %d\n", subkey->GetString(), info.level);
			} else if (FStrEq(name, "Bodygroup")) {
				info.bodygroup = subkey->GetInt();
			//	DevMsg("Level \"%s\" --> %d\n", subkey->GetString(), info.level);
			} else if (FStrEq(name, "Skin")) {
				info.skin = subkey->GetInt();
			//	DevMsg("Level \"%s\" --> %d\n", subkey->GetString(), info.level);
			} else if (FStrEq(name, "Position")) {
				FOR_EACH_SUBKEY(subkey, subsub) {
					const char *name = subsub->GetName();
					float value      = subsub->GetFloat();
					
					if (FStrEq(name, "X")) {
						info.origin.x = value;
					} else if (FStrEq(name, "Y")) {
						info.origin.y = value;
					} else if (FStrEq(name, "Z")) {
						info.origin.z = value;
					} else if (FStrEq(name, "Pitch")) {
						info.angles.x = value;
					} else if (FStrEq(name, "Yaw")) {
						info.angles.y = value;
					} else if (FStrEq(name, "Roll")) {
						info.angles.z = value;
					} else {
						Warning("Unknown key \'%s\' in SentryGun Position sub-block.\n", name);
					}
				}
				info.use_hint = false;
			} else {
				Warning("Unknown key \'%s\' in SentryGun block.\n", name);
			}
		}
		
		bool fail = false;
		if (info.use_hint && info.hints.empty()) {
			Warning("Missing HintName key or Position block in SentryGun block.\n");
			fail = true;
		}
		if (info.level == -1) {
			Warning("Missing Level key in SentryGun block.\n");
			fail = true;
		}
		if (fail) return;
		
		DevMsg("Wave %08x: add SentryGun\n", (uintptr_t)wave);
		waves[wave].sentryguns.push_back(info);
	}
	
	void Parse_HalloweenBoss(CWave *wave, KeyValues *kv)
	{
		BossInfo info;
		
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			if (FStrEq(name, "BossType")) {
				if (FStrEq(subkey->GetString(), "HHH")) {
					info.type = CHalloweenBaseBoss::HEADLESS_HATMAN;
				} else if (FStrEq(subkey->GetString(), "MONOCULUS")) {
					info.type = CHalloweenBaseBoss::EYEBALL_BOSS;
				} else if (FStrEq(subkey->GetString(), "Merasmus")) {
					info.type = CHalloweenBaseBoss::MERASMUS;
				} else {
					Warning("Invalid value \'%s\' for BossType key in HalloweenBoss block.\n", subkey->GetString());
				}
			//	DevMsg("BossType \"%s\" --> %d\n", subkey->GetString(), info.type);
			} else if (FStrEq(name, "TeamNum")) {
				info.teamnum = subkey->GetInt();
			//	DevMsg("TeamNum \"%s\" --> %d\n", subkey->GetString(), info.teamnum);
			} else if (FStrEq(name, "Health")) {
				info.health = Max(1, subkey->GetInt());
			//	DevMsg("Health \"%s\" --> %d\n", subkey->GetString(), info.health);
			} else if (FStrEq(name, "Delay")) {
				info.delay = Max(0.0f, subkey->GetFloat());
			//	DevMsg("Delay \"%s\" --> %.1f\n", subkey->GetString(), info.delay);
			} else if (FStrEq(name, "Position")) {
				FOR_EACH_SUBKEY(subkey, subsub) {
					const char *name = subsub->GetName();
					float value      = subsub->GetFloat();
					
					if (FStrEq(name, "X")) {
						info.origin.x = value;
					} else if (FStrEq(name, "Y")) {
						info.origin.y = value;
					} else if (FStrEq(name, "Z")) {
						info.origin.z = value;
					} else {
						Warning("Unknown key \'%s\' in HalloweenBoss Position sub-block.\n", name);
					}
				}
			} else {
				Warning("Unknown key \'%s\' in HalloweenBoss block.\n", name);
			}
		}
		
		bool fail = false;
		if (info.type == CHalloweenBaseBoss::INVALID) {
			Warning("Missing BossType key in HalloweenBoss block.\n");
			fail = true;
		}
		if (info.teamnum != TF_TEAM_HALLOWEEN_BOSS) {
			if (info.type == CHalloweenBaseBoss::EYEBALL_BOSS) {
				if (info.teamnum != TF_TEAM_RED && info.teamnum != TF_TEAM_BLUE) {
					Warning("Invalid value for TeamNum key in HalloweenBoss block: MONOCULUS must be team 5 or 2 or 3.\n");
					fail = true;
				}
			} else {
				Warning("Invalid value for TeamNum key in HalloweenBoss block: HHH and Merasmus must be team 5.\n");
				fail = true;
			}
		}
		if (fail) return;
		
		DevMsg("Wave %08x: add HalloweenBoss\n", (uintptr_t)wave);
		waves[wave].bosses.push_back(info);
	}
	
	void Parse_SoundLoop(CWave *wave, KeyValues *kv)
	{
		if (!waves[wave].sound_loops.empty()) {
			Warning("Multiple \'SoundLoop\' blocks found in the same Wave!\n");
			return;
		}
		
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			if (FStrEq(name, "SoundFile")) {
				waves[wave].sound_loops[subkey->GetString()]=0;
			} else {
				waves[wave].sound_loops[name]=subkey->GetFloat();
			}
		}
	}
	
	void Parse_PlayerAttributes(CWave *wave, KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			int classname = 0;
			for(int i=1; i < 10; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			if (classname == 0) {
				if (GetItemSchema()->GetAttributeDefinitionByName(subkey->GetName()) != nullptr) {
					waves[wave].player_attributes[subkey->GetName()] = subkey->GetFloat();
					DevMsg("Parsed attribute %s %f\n", subkey->GetName(),subkey->GetFloat());
				}
			}
			else {
				waves[wave].defined_class_attributes = true;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					if (GetItemSchema()->GetAttributeDefinitionByName(subkey2->GetName()) != nullptr) {
						waves[wave].player_attributes_class[classname][subkey2->GetName()] = subkey2->GetFloat();
						DevMsg("Parsed attribute %s %f\n", subkey2->GetName(),subkey2->GetFloat());
					}
				}
			}
			
		}
		
		DevMsg("Parsed attributes\n");
	}
	void Parse_PlayerAddCond(CWave *wave, KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			int classname = 0;
			for(int i=1; i < 10; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			if (classname == 0) {
				const char *name = subkey->GetName();
				ETFCond cond = (ETFCond)-1;
				if (FStrEq(name, "Index"))
					cond = (ETFCond)subkey->GetInt();
				else if (FStrEq(name, "Name"))
					cond = GetTFConditionFromName(subkey->GetString());

				if (cond == -1)
					Warning("Unrecognized condition name \"%s\" in AddCond block.\n", subkey->GetString());
				else
					waves[wave].addconds.push_back(cond);
			}
			else {
				waves[wave].defined_class_attributes = true;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					const char *name = subkey2->GetName();
					ETFCond cond = (ETFCond)-1;
					if (FStrEq(name, "Index"))
						cond = (ETFCond)subkey2->GetInt();
					else if (FStrEq(name, "Name"))
						cond = GetTFConditionFromName(subkey2->GetString());

					if (cond == -1)
						Warning("Unrecognized condition name \"%s\" in AddCond block.\n", subkey2->GetString());
					else
						waves[wave].addconds_class[classname].push_back(cond);
				}
			}
		}
		
		DevMsg("Parsed addcond\n");
	}

	DETOUR_DECL_MEMBER(bool, CWave_Parse, KeyValues *kv)
	{
		auto wave = reinterpret_cast<CWave *>(this);
		
	//	DevMsg("CWave::Parse\n");
		
		std::vector<KeyValues *> del_kv;
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			bool del = true;
			if (FStrEq(name, "Explanation")) {
				Parse_Explanation(wave, subkey);
			} else if (FStrEq(name, "SentryGun")) {
				Parse_SentryGun(wave, subkey);
			} else if (FStrEq(name, "HalloweenBoss")) {
				Parse_HalloweenBoss(wave, subkey);
			} else if (FStrEq(name, "SoundLoop")) {
				Parse_SoundLoop(wave, subkey);
			} else if (FStrEq(name, "PlayerAttributes")) {
				Parse_PlayerAttributes(wave, subkey);
			} else if (FStrEq(name, "RedTeamWipeCausesWaveLoss")) {
				waves[wave].red_team_wipe_causes_wave_loss = subkey->GetBool();
			} else if (FStrEq(name, "BlueTeamWipeCausesWaveLoss")) {
				waves[wave].blue_team_wipe_causes_wave_loss = subkey->GetBool();
			} else if (FStrEq(name, "FinishingWaveCausesWaveLoss")) {
				waves[wave].finishing_wave_causes_wave_loss = subkey->GetBool();
			} else if (FStrEq(name, "FinishingWaveAndPlayerWipeCausesWaveLoss")) {
				waves[wave].finishing_wave_and_player_wipe_causes_wave_loss = subkey->GetBool();
			} else if (FStrEq(name, "SpawnTemplate")) {
				PointTemplateInfo info =Parse_SpawnTemplate(subkey);
				if (info.templ != nullptr)
					waves[wave].templ.push_back(info);
			} else if (FStrEq(name, "PlayerAddCond")) {
				Parse_PlayerAddCond(wave, subkey);
			} else {
				del = false;
			}
			
			if (del) {
	//			DevMsg("Key \"%s\": processed, will delete\n", name);
				del_kv.push_back(subkey);
			} else {
	//			DevMsg("Key \"%s\": passthru\n", name);
			}
		}
		
		for (auto subkey : del_kv) {
	//		DevMsg("Deleting key \"%s\"\n", subkey->GetName());
			kv->RemoveSubKey(subkey);
			subkey->deleteThis();
		}
		
		return DETOUR_MEMBER_CALL(CWave_Parse)(kv);
	}
	
	ConVar sig_text_print_speed("sig_text_print_speed", "4");
	void ParseColorsAndPrint(const char *line, float gameTextDelay, int &linenum, CTFPlayer* player = nullptr)
	{
		std::vector<char> output;

		std::vector<char> output_nocolor;
		int color = 0xFFFFFF;

		char num[7];
		int colorcode_idx=0;
		
		bool hasText = false;
		/* always start with reset so that colors will work properly */
		output.push_back('\x01');
		
		bool in_braces = false;
		int brace_idx = 0;
		
		int i = 0;
		char c;
		while ((c = line[i]) != '\0') {
			hasText |= isalnum(c);

			if (in_braces) {
				if (c == '}') {
					const char *brace_str = line + brace_idx;
					int brace_len = i - brace_idx;
					
					if (V_strnicmp(brace_str, "Reset", brace_len) == 0) {
						output.push_back('\x01');
					} else if (V_strnicmp(brace_str, "Blue", brace_len) == 0) {
						output.insert(output.end(), {'\x07', '9', '9', 'c', 'c', 'f', 'f'});
						color = 0x99ccff;
					} else if (V_strnicmp(brace_str, "Red", brace_len) == 0) {
						output.insert(output.end(), {'\x07', 'f', 'f', '3', 'f', '3', 'f'});
						color = 0xff3f3f;
					} else if (V_strnicmp(brace_str, "Green", brace_len) == 0) {
						output.insert(output.end(), {'\x07', '9', '9', 'f', 'f', '9', '9'});
						color = 0x99ff99;
					} else if (V_strnicmp(brace_str, "DarkGreen", brace_len) == 0) {
						output.insert(output.end(), {'\x07', '4', '0', 'f', 'f', '4', '0'});
						color = 0x40ff40;
					} else if (V_strnicmp(brace_str, "Yellow", brace_len) == 0) {
						output.insert(output.end(), {'\x07', 'f', 'f', 'b', '2', '0', '0'});
						color = 0xffb200;
					} else if (V_strnicmp(brace_str, "Grey", brace_len) == 0) {
						output.insert(output.end(), {'\x07', 'c', 'c', 'c', 'c', 'c', 'c'});
						color = 0xcccccc;
					} else {
						/* RGB hex code */
						if (brace_len >= 6) {
							output.push_back('\x07');
							output.push_back(brace_str[0]);
							output.push_back(brace_str[1]);
							output.push_back(brace_str[2]);
							output.push_back(brace_str[3]);
							output.push_back(brace_str[4]);
							output.push_back(brace_str[5]);
							memcpy(num, brace_str,6);
							color = strtol(num, nullptr, 16);
						}
					}
					
					in_braces = false;
				}
			} else {
				if (c == '{') {
					in_braces = true;
					brace_idx = i + 1;
				} else {
					output.push_back(c);
					if (c == '\x07')
						colorcode_idx = i+1;

					if (colorcode_idx != 0) {
						if (i >= colorcode_idx) {
							if (i - colorcode_idx < 6) {
								num[i - colorcode_idx] = c;
							}
							else {
								color = strtol(num, nullptr, 16);
								colorcode_idx = 0;
							}
						}
					}
					else
						output_nocolor.push_back(c);
				}
			}

			++i;
		}
		
		/* append an extra space at the end to make empty lines show up */
		output.push_back(' ');
		output.push_back('\n');
		output.push_back('\0');
		output_nocolor.push_back('\0');
		if (hasText && sig_text_print_speed.GetFloat() > 0.0f) {
			// The first line is always drawn before others so it makes sense to only then create a new game_text entity if its missing
			if (linenum == 0 && servertools->FindEntityByName(nullptr, "wave_explanation_text") == nullptr) {
				CBaseEntity *textent = CreateEntityByName("game_text");
				servertools->SetKeyValue(textent, "targetname", "wave_explanation_text");
				servertools->SetKeyValue(textent, "channel", "0");
				servertools->SetKeyValue(textent, "effect", "2");
				servertools->SetKeyValue(textent, "fadeout", "0.5");
				servertools->SetKeyValue(textent, "fxtime", "0.5");
				servertools->SetKeyValue(textent, "holdtime", "2.5");
				servertools->SetKeyValue(textent, "x", "-1");
				servertools->SetKeyValue(textent, "y", "0.25");
				servertools->SetKeyValue(textent, "spawnflags", "0");
				textent->Spawn();
				textent->Activate();
			}
			//textent->AcceptInput("Display",player,player,variant,-1);
			CEventQueue &que = g_EventQueue;

			variant_t variant;
			
			//servertools->SetKeyValue(textent, "color", CFmtStr("%d %d %d", ((color >> 16) & 255), ((color >> 8) & 255), (color & 255)));
			//servertools->SetKeyValue(textent, "color2", CFmtStr("%d %d %d", ((color >> 16) & 255), ((color >> 8) & 255), (color & 255)));
			bool hasPlayer = player != nullptr;
			float delay = gameTextDelay + linenum * sig_text_print_speed.GetFloat() + (hasPlayer ? 0.5f : 0.0f);
			variant.SetString(AllocPooledString(CFmtStr("color %d %d %d", ((color >> 16) & 255), ((color >> 8) & 255), (color & 255))));
			que.AddEvent("wave_explanation_text","addoutput",variant,delay, player,player,-1);
			
			variant.SetString(AllocPooledString(CFmtStr("color2 %d %d %d", ((color >> 16) & 255), ((color >> 8) & 255), (color & 255))));
			que.AddEvent("wave_explanation_text","addoutput",variant,delay, player,player,-1);

			variant.SetString(AllocPooledString(CFmtStr("message %s", output_nocolor.data())));
			que.AddEvent("wave_explanation_text","addoutput",variant,delay, player,player,-1);

			variant.SetString(AllocPooledString(CFmtStr("fadein %f", 1.0f/output_nocolor.size() * sig_text_print_speed.GetFloat() * 0.25f)));
			que.AddEvent("wave_explanation_text","addoutput",variant,delay, player,player,-1);

			variant.SetString(AllocPooledString(CFmtStr("holdtime %f", 0.75f * sig_text_print_speed.GetFloat() - 0.5f )));
			que.AddEvent("wave_explanation_text","addoutput",variant,delay, player,player,-1);

			if (!hasPlayer)
			for (int i = 1; i <= gpGlobals->maxClients; ++i) {
				CBasePlayer *baseplayer = UTIL_PlayerByIndex(i);

				que.AddEvent("wave_explanation_text","display",variant,delay + 0.01f, baseplayer,baseplayer,-1);
			}

			que.AddEvent("wave_explanation_text","display",variant,delay + 0.01f, player,player,-1);

			linenum++;
		}

		if (!player)
			PrintToChatAll(output.data());
		else
			PrintToChat(output.data(), player);
	}
	float last_explanation_time = 0.0f;

	std::set<CHandle<CTFPlayer>> saw_explanation;
	void ShowWaveExplanation(bool success, CTFPlayer *player = nullptr)
	{
		
		CWave *wave = g_pPopulationManager->GetCurrentWave();
		if (wave == nullptr) return;

		if (player == nullptr) {
			if (gpGlobals->curtime - last_explanation_time < 0.5f )
				return;
			else
				last_explanation_time = gpGlobals->curtime;
				
			saw_explanation.clear();
			ForEachTFPlayer([&](CTFPlayer *playerl){
				if (!playerl->IsFakeClient()) {
					saw_explanation.insert(playerl);
				}
			});
		}
		/* wave will be null after game is won and in other corner cases */
		
		if (player != nullptr && saw_explanation.find(player) != saw_explanation.end()) return;
		
		if (player != nullptr) {
			saw_explanation.insert(player);
		}

		const auto& explanation = waves[wave].explanation;

		if (explanation.size() > 0) {

			int linenum = 0;
			for (const auto& line : explanation) {
				ParseColorsAndPrint(line.c_str(), success ? 15.0f : 1.0f, linenum, player);
			}
		}
	}
	DETOUR_DECL_MEMBER(void, CTFPlayer_HandleCommand_JoinClass, const char *pClassName, bool b1)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		bool show_wave_expl = !player->IsBot() && player->GetPlayerClass()->GetClassIndex() == TF_CLASS_UNDEFINED;
		DETOUR_MEMBER_CALL(CTFPlayer_HandleCommand_JoinClass)(pClassName, b1);
		if (show_wave_expl && TFGameRules()->IsMannVsMachineMode())
			ShowWaveExplanation(false, player);
	}

	void OnWaveBegin(bool success) {
		CWave *wave = g_pPopulationManager->GetCurrentWave();
		if (wave == nullptr)
			return;

		ShowWaveExplanation(success);

		auto data = waves[wave];
		if (!data.t_waveinit.HasStarted()) {
			data.t_waveinit.Start();
		}

		// Remove old force items
		/*ForEachTFPlayer([&](CTFPlayer *player) {
			if (player->IsBot() || !player->IsAlive())
				return;
			
			player->
		}*/
	}
	int rc_JumpToWave = 0;
	DETOUR_DECL_MEMBER(void, CPopulationManager_JumpToWave, unsigned int wave, float f1)
	{
		DevMsg("[%8.3f] JumpToWave\n", gpGlobals->curtime);

		rc_JumpToWave++;
		DETOUR_MEMBER_CALL(CPopulationManager_JumpToWave)(wave, f1);
		rc_JumpToWave--;

		OnWaveBegin(false);
	}
	
	DETOUR_DECL_MEMBER(void, CPopulationManager_WaveEnd, bool b1)
	{
		//DevMsg("[%8.3f] WaveEnd\n", gpGlobals->curtime);
		CWave *wave = g_pPopulationManager->GetCurrentWave();
		if (wave != nullptr)
			WaveCleanup(wave);
		DETOUR_MEMBER_CALL(CPopulationManager_WaveEnd)(b1);
		OnWaveBegin(true);
	}
	
	DETOUR_DECL_MEMBER(void, CMannVsMachineStats_RoundEvent_WaveEnd, bool success)
	{
		DETOUR_MEMBER_CALL(CMannVsMachineStats_RoundEvent_WaveEnd)(success);
		if (!success && rc_JumpToWave == 0) {
			//DevMsg("[%8.3f] RoundEvent_WaveEnd\n", gpGlobals->curtime);
			OnWaveBegin(false);
		}
	}
	
	
	CObjectSentrygun *SpawnSentryGun(const Vector& origin, const QAngle& angles, int teamnum, int level, bool is_mini, int health, int skin, int bodygroup)
	{
		auto sentry = rtti_cast<CObjectSentrygun *>(CreateEntityByName("obj_sentrygun"));
		if (sentry == nullptr) {
			Warning("SpawnSentryGun: CreateEntityByName(\"obj_sentrygun\") failed\n");
			return nullptr;
		}
		
	//	DevMsg("[%8.3f] SpawnSentryGun: [hint #%d \"%s\"] [teamnum %d] [level %d]\n",
	//		gpGlobals->curtime, ENTINDEX(sg_info.hint), STRING(sg_info.hint->GetEntityName()), sg_info.teamnum, sg_info.level);
		
		sentry->SetAbsOrigin(origin);
		sentry->SetAbsAngles(angles);
		
		sentry->Spawn();
		
		sentry->ChangeTeam(teamnum);
		sentry->m_nDefaultUpgradeLevel = level - 1;
		
		if (is_mini) {
			sentry->SetModelScale(0.75f);
			sentry->m_bMiniBuilding = true;
			sentry->m_nSkin += 2;
		}
		
		sentry->InitializeMapPlacedObject();

		sentry->m_nBody = bodygroup;

		sentry->m_nSkin = skin;

		if (health != -1) {
			sentry->SetMaxHealth(health);
			sentry->SetHealth((float)health);
		}
		
		DevMsg("SpawnSentryGun: #%d, %08x, level %d, health %d, maxhealth %d\n",
			ENTINDEX(sentry), (uintptr_t)sentry, level, sentry->GetHealth(), sentry->GetMaxHealth());
		
		return sentry;
	}
	
	void SpawnSentryGuns(SentryGunInfo& info)
	{
		info.spawned = true;
		
		if (info.use_hint && info.hints.empty()) {
			Warning("SpawnSentryGuns: info.hints.empty()\n");
			return;
		}
		
		if (info.use_hint) {
			for (const auto& hint : info.hints) {
				info.sentry = SpawnSentryGun(hint->GetAbsOrigin(), hint->GetAbsAngles(), info.teamnum, info.level, info.is_mini, info.health, info.skin, info.bodygroup);
			}
		} else {
			info.sentry = SpawnSentryGun(info.origin, info.angles, info.teamnum, info.level, info.is_mini, info.health, info.skin, info.bodygroup);
		}
	}
	
	
	void SpawnBoss(BossInfo& info)
	{
		info.spawned = true;
		
		CHalloweenBaseBoss *boss = CHalloweenBaseBoss::SpawnBossAtPos(info.type, info.origin, info.teamnum, nullptr);
		if (boss == nullptr) {
			Warning("SpawnBoss: CHalloweenBaseBoss::SpawnBossAtPos(type %d, teamnum %d) failed\n", info.type, info.teamnum);
			return;
		}
		
		if (info.health > 0) {
			boss->SetMaxHealth(info.health);
			boss->SetHealth   (info.health);
		}
		
		info.boss = boss;
	}
	
	std::string soundloop_active;

	void StopSoundLoop()
	{
		ConColorMsg(Color(0xff, 0x00, 0x00, 0xff), "[SoundLoop] StopSoundLoop \"%s\"\n", soundloop_active.c_str());
		
		if (TFGameRules() != nullptr) {
			TFGameRules()->BroadcastSound(SOUND_FROM_LOCAL_PLAYER, soundloop_active.c_str(), SND_STOP);
		}
		
		soundloop_active.clear();
	}
	
	void StartSoundLoop(const std::string& filename)
	{
		if (!soundloop_active.empty()) {
			StopSoundLoop();
		}
		
		ConColorMsg(Color(0x00, 0xff, 0x00, 0xff), "[SoundLoop] StartSoundLoop \"%s\"\n", filename.c_str());
		
		/* if filename is explicitly "", then don't play anything */
		if (TFGameRules() != nullptr && filename != "") {
			TFGameRules()->BroadcastSound(SOUND_FROM_LOCAL_PLAYER, filename.c_str(), SND_NOFLAGS);
			soundloop_active = filename;
		}
	}

	void SelectLoopSound(WaveData &data) {
		if (!data.sound_loops.empty()) {
			auto sound_loop = select_random(data.sound_loops.begin(),data.sound_loops.end());
			StartSoundLoop(sound_loop->first);
			if (sound_loop->second > 0.0f)
				data.sound_time_end = data.t_wavestart.GetElapsedTime() + sound_loop->second;
			else
				data.sound_time_end = data.t_wavestart.GetElapsedTime() + 99999;
		}
	}
	
	DETOUR_DECL_MEMBER(void, CWave_ActiveWaveUpdate)
	{
		auto wave = reinterpret_cast<CWave *>(this);
		
		auto it = waves.find(wave);
		if (it == waves.end()) {
			DETOUR_MEMBER_CALL(CWave_ActiveWaveUpdate)();
			return;
		}
		WaveData& data = (*it).second;
		
		if (!data.t_wavestart.HasStarted()) {
			data.t_wavestart.Start();

		}
		
		/* since we are pre-detour and ActiveWaveUpdate only happens in RND_RUNNING, we are safe to skip the check */
		if (data.red_team_wipe_causes_wave_loss/* && TFGameRules()->State_Get() == GR_STATE_RND_RUNNING*/) {
			CTFTeam *team_red = TFTeamMgr()->GetTeam(TF_TEAM_RED);
			if (team_red != nullptr && team_red->GetNumPlayers() != 0) {
				int num_red_humans       = 0;
				int num_red_humans_alive = 0;
				
				ForEachTFPlayerOnTeam(TFTeamMgr()->GetTeam(TF_TEAM_RED), [&](CTFPlayer *player){
					if (player->IsBot()) return;
					
					++num_red_humans;
					if (player->IsAlive()) {
						++num_red_humans_alive;
					}
				});
				
				/* if red team actually contains zero humans, then don't do anything */
				if (num_red_humans > 0 && num_red_humans_alive == 0) {
					/* not entirely sure what effect the win reason parameter has (if it even has an effect at all) */
					TFGameRules()->SetWinningTeam(TF_TEAM_BLUE, WINREASON_OPPONENTS_DEAD, true, false);
				}
			}
		}
		
		if (data.blue_team_wipe_causes_wave_loss/* && TFGameRules()->State_Get() == GR_STATE_RND_RUNNING*/) {
			CTFTeam *team_blue = TFTeamMgr()->GetTeam(TF_TEAM_BLUE);
			if (team_blue != nullptr && team_blue->GetNumPlayers() != 0) {
				int num_blue_humans       = 0;
				int num_blue_humans_alive = 0;
				
				ForEachTFPlayerOnTeam(TFTeamMgr()->GetTeam(TF_TEAM_BLUE), [&](CTFPlayer *player){
					if (player->IsBot()) return;
					
					++num_blue_humans;
					if (player->IsAlive()) {
						++num_blue_humans_alive;
					}
				});
				
				/* if red team actually contains zero humans, then don't do anything */
				if (num_blue_humans > 0 && num_blue_humans_alive == 0) {
					/* not entirely sure what effect the win reason parameter has (if it even has an effect at all) */
					TFGameRules()->SetWinningTeam(TF_TEAM_RED, WINREASON_OPPONENTS_DEAD, true, false);
				}
			}
		}

		if (data.defined_class_attributes || data.player_attributes.size() > 0 || data.addconds.size() > 0)
			ForEachTFPlayer([&](CTFPlayer *player){
				if (player->IsBot()) return;
				if (!player->IsAlive()) return;
				for(auto it = data.player_attributes.begin(); it != data.player_attributes.end(); ++it){
					
					player->AddCustomAttribute(it->first.c_str(),it->second, 1.0f);
				}
				int classname = player->GetPlayerClass()->GetClassIndex();

				for(auto it = data.player_attributes_class[classname].begin(); it != data.player_attributes_class[classname].end(); ++it){
						player->AddCustomAttribute(it->first.c_str(),it->second, 7200.0f);
					}
				for(auto cond : data.addconds){
					if (!player->m_Shared->InCond(cond)){
						player->m_Shared->AddCond(cond,-1.0f);
					}
				}
				for(auto cond : data.addconds_class[classname]){
					if (!player->m_Shared->InCond(cond)){
						player->m_Shared->AddCond(cond,-1.0f);
					}
				}
			});
			

		// ^^^^   PRE-DETOUR ===================================================
		
		DETOUR_MEMBER_CALL(CWave_ActiveWaveUpdate)();
		
		// vvvv  POST-DETOUR ===================================================
		
		for (auto& info : data.sentryguns) {
			if (!info.spawned && !data.t_wavestart.IsLessThen(info.delay)) {
				SpawnSentryGuns(info);
			}
		}
		
		for (auto& info : data.bosses) {
			if (!info.spawned && !data.t_wavestart.IsLessThen(info.delay)) {
				SpawnBoss(info);
			}
		}
		for (auto it1 = data.templ.begin(); it1 != data.templ.end(); ++it1) {
			if(!data.t_wavestart.IsLessThen(it1->delay)){
				auto templ_inst = it1->SpawnTemplate(nullptr);
				if (templ_inst != nullptr) {
					templ_inst->is_wave_spawned = true;
					data.templ_inst.push_back(templ_inst);
				}
				data.templ.erase(it1);
				it1--;
			}
		}
		if (!data.t_wavestart.IsLessThen(data.sound_time_end)) {
			SelectLoopSound(data);
		}
	}
	
	DETOUR_DECL_MEMBER(bool, CWave_IsDoneWithNonSupportWaves)
	{
		bool done = DETOUR_MEMBER_CALL(CWave_IsDoneWithNonSupportWaves)();
		
		auto wave = reinterpret_cast<CWave *>(this);
		
		auto it = waves.find(wave);
		if (it != waves.end()) {
			WaveData& data = (*it).second;
			
			for (auto& info : data.bosses) {
				if (info.spawned && info.boss != nullptr && info.boss->IsAlive()) {
					return false;
				}
			}

			if (done && (data.finishing_wave_causes_wave_loss || data.finishing_wave_and_player_wipe_causes_wave_loss)) {
				if (!data.finishing_wave_causes_wave_loss) {
					
					int playercount = 0;
					int aliveplayercount = 0;
					ForEachTFPlayer([&](CTFPlayer *playerl){
						if (!playerl->IsFakeClient() && playerl->GetTeamNumber() > 1) {
							playercount +=1;
							if (playerl->IsAlive())
								aliveplayercount +=1;
						}
					});
					if (playercount > 0 && aliveplayercount == 0) {
						TFGameRules()->SetWinningTeam(TF_TEAM_RED, WINREASON_OPPONENTS_DEAD, true, false);
						done = false;
					}
				}
				else{
					TFGameRules()->SetWinningTeam(TF_TEAM_RED, WINREASON_OPPONENTS_DEAD, true, false);
					done = false;
				}
			}
		}
		
		return done;
	}
	CWave *last_wave;
	DETOUR_DECL_MEMBER(void, CTeamplayRoundBasedRules_State_Enter, gamerules_roundstate_t newState)
	{
		auto oldState = TeamplayRoundBasedRules()->State_Get();
		
		ConColorMsg(Color(0xff, 0x00, 0xff, 0xff),
			"[SoundLoop] CTeamplayRoundBasedRules: %s -> %s\n",
			GetRoundStateName(oldState), GetRoundStateName(newState));
		
		
		if (oldState != GR_STATE_RND_RUNNING && newState == GR_STATE_RND_RUNNING) {
			if (TFGameRules()->IsMannVsMachineMode()) {
				last_wave = g_pPopulationManager->GetCurrentWave();
				auto it = waves.find(last_wave);
				if (it != waves.end()) {
					WaveData& data = (*it).second;
					SelectLoopSound(data);
				}
			}
		} else if (oldState == GR_STATE_RND_RUNNING && newState != GR_STATE_RND_RUNNING) {
			if (last_wave != nullptr) {
				auto it = waves.find(last_wave); 
				if (it != waves.end()) {
					WaveData& data = (*it).second;

					if (data.defined_class_attributes || data.player_attributes.size() > 0 || data.addconds.size() > 0) {
						ForEachTFPlayer([&](CTFPlayer *player){
							if (player->IsBot()) return;
							for(auto it = data.player_attributes.begin(); it != data.player_attributes.end(); ++it){
								player->RemoveCustomAttribute(it->first.c_str());
							}
							int classname = player->GetPlayerClass()->GetClassIndex();

							for(auto it = data.player_attributes_class[classname].begin(); it != data.player_attributes_class[classname].end(); ++it){
									player->RemoveCustomAttribute(it->first.c_str());
								}
							for(auto cond : data.addconds){
								if (player->m_Shared->InCond(cond)){
									player->m_Shared->RemoveCond(cond);
								}
							}
							for(auto cond : data.addconds_class[classname]){
								if (player->m_Shared->InCond(cond)){
									player->m_Shared->RemoveCond(cond);
								}
							}
						});
					}
				}
			}
			StopSoundLoop();
		}
		
		DETOUR_MEMBER_CALL(CTeamplayRoundBasedRules_State_Enter)(newState);
	}
	
	class CMod : public IMod, public IModCallbackListener
	{
	public:
		CMod() : IMod("Pop:Wave_Extensions")
		{
			MOD_ADD_DETOUR_MEMBER(CWave_dtor0, "CWave::~CWave [D0]");
			MOD_ADD_DETOUR_MEMBER(CWave_dtor2, "CWave::~CWave [D2]");
			
			MOD_ADD_DETOUR_MEMBER(CWave_Parse, "CWave::Parse");
			
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_JumpToWave,          "CPopulationManager::JumpToWave");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_WaveEnd,             "CPopulationManager::WaveEnd");
			MOD_ADD_DETOUR_MEMBER(CMannVsMachineStats_RoundEvent_WaveEnd, "CMannVsMachineStats::RoundEvent_WaveEnd");
			
			MOD_ADD_DETOUR_MEMBER(CWave_ActiveWaveUpdate,          "CWave::ActiveWaveUpdate");
			MOD_ADD_DETOUR_MEMBER(CWave_IsDoneWithNonSupportWaves, "CWave::IsDoneWithNonSupportWaves");
			
			MOD_ADD_DETOUR_MEMBER(CTeamplayRoundBasedRules_State_Enter, "CTeamplayRoundBasedRules::State_Enter");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_HandleCommand_JoinClass,                  "CTFPlayer::HandleCommand_JoinClass");
			
		}
		
		virtual void OnUnload() override
		{
			waves.clear();
			StopSoundLoop();
		}
		
		virtual void OnEnable() override
		{
		}

		virtual void OnDisable() override
		{
			waves.clear();
			StopSoundLoop();
		}
		
		virtual bool ShouldReceiveCallbacks() const override { return this->IsEnabled(); }
		
		virtual void LevelInitPreEntity() override
		{
			waves.clear();
			StopSoundLoop();
			last_explanation_time = 0.0f;
		}
		
		virtual void LevelShutdownPostEntity() override
		{
			waves.clear();
			StopSoundLoop();
		}

		/*virtual void FrameUpdatePostEntityThink() override
		{
			if(g_pPopulationManager != nullptr) {
				CWave *wave = g_pPopulationManager->GetCurrentWave();
				if (wave != nullptr) {
					for (int i = 0; i < 32; i++) {
						CBasePlayer *player = UTIL_PlayerByIndex(i);
						if (player == nullptr) continue;
						if (player->IsBot()) continue;
						
						float &progress = waves[wave].player_explanation_progress[i];
						
						if (progress > 0) {
							
						}
					}
				}
			}
		}*/
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_pop_wave_extensions", "0", FCVAR_NOTIFY,
		"Mod: enable extended KV in CWave::Parse",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
	
	
	class CKVCond_Wave : public IKVCond
	{
	public:
		virtual bool operator()() override
		{
			return s_Mod.IsEnabled();
		}
	};
	CKVCond_Wave cond;
}


/* wave explanation TODO:

each time a wave starts, reset some tracking info
- keep track of which steam IDs have seen the explanation and which haven't

if a new player connects, and the wave has an explanation, and they haven't seen it,
then show it *just* to them, a few seconds after they pick a class






*/
