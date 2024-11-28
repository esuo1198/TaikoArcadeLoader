// ReSharper disable CppTooWideScopeInitStatement
#include "helpers.h"
#include "../patches.h"
#include <map>

namespace patches::JPN39 {
int language = 0;
HOOK_DYNAMIC (char, AMFWTerminate, i64) {
    LogMessage (LogLevel::HOOKS, "AMFWTerminate was called");
    return 0;
}

HOOK_DYNAMIC (i64, curl_easy_setopt, i64 a1, i64 a2, i64 a3, i64 a4, i64 a5) {
    LogMessage (LogLevel::HOOKS, "Garmc curl_easy_setopt was called");
    originalcurl_easy_setopt (a1, 64, 0, 0, 0);
    originalcurl_easy_setopt (a1, 81, 0, 0, 0);
    return originalcurl_easy_setopt (a1, a2, a3, a4, a5);
}

FUNCTION_PTR (i64, GetPlayDataManagerRef, ASLR (0x140024AC0), i64);

i64 lua_State = 0;
HOOK (i64, luaL_newstate, PROC_ADDRESS ("lua51.dll", "luaL_newstate")) { return lua_State = originalluaL_newstate (); }
FUNCTION_PTR (void, lua_settop, PROC_ADDRESS ("lua51.dll", "lua_settop"), i64, i32);
FUNCTION_PTR (void, lua_replace, PROC_ADDRESS ("lua51.dll", "lua_replace"), i64, i32);
FUNCTION_PTR (void, lua_pushcclosure, PROC_ADDRESS ("lua51.dll", "lua_pushcclosure"), i64, i64, i32);
FUNCTION_PTR (void, lua_pushboolean, PROC_ADDRESS ("lua51.dll", "lua_pushboolean"), i64, i32);
FUNCTION_PTR (const char *, lua_pushstring, PROC_ADDRESS ("lua51.dll", "lua_pushstring"), i64, const char *);
FUNCTION_PTR (i32, lua_toboolean, PROC_ADDRESS ("lua51.dll", "lua_toboolean"), i64, i32);
FUNCTION_PTR (const char *, lua_tolstring, PROC_ADDRESS ("lua51.dll", "lua_tolstring"), u64, i32, size_t *);
FUNCTION_PTR (i32, lua_pcall, PROC_ADDRESS ("lua51.dll", "lua_pcall"), i64, i32, i32, i32);
FUNCTION_PTR (i32, luaL_loadstring, PROC_ADDRESS ("lua51.dll", "luaL_loadstring"), i64, const char *);
#define LUA_MULTRET         (-1)
#define luaL_dostring(L, s) (luaL_loadstring (L, s) || lua_pcall (L, 0, LUA_MULTRET, 0))

FUNCTION_PTR (u64, RefTestModeMain, ASLR (0x1400337C0), u64);
FUNCTION_PTR (u64, RefPlayDataManager, ASLR (0x140024AC0), u64);
FUNCTION_PTR (i64, GetUserCount, ASLR (0x1403F1020), u64);

i64
lua_pushbool (const i64 a1, const bool val) {
    lua_settop (a1, 0);
    lua_pushboolean (a1, val);
    return 1;
}

u64 appAccessor       = 0;
u64 componentAccessor = 0;
HOOK (i64, DeviceCheck, ASLR (0x140464FC0), i64 a1, i64 a2, i64 a3) {
    LogMessage (LogLevel::HOOKS, "DeviceCheck was called");
    TestMode::SetupAccessor (a3, RefTestModeMain);
    componentAccessor = a2;
    return originalDeviceCheck (a1, a2, a3);
}

i64
GetUserStatus () {
    if (appAccessor) {
        if (const u64 playDataManager = RefPlayDataManager (appAccessor)) return GetUserCount (playDataManager);
    }
    return -1;
}

HOOK (i64, AvailableMode_Collabo024, ASLR (0x1402DE710), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo024 was called");
    if (const int tournamentMode = TestMode::ReadTestModeValue (L"TournamentMode"); tournamentMode == 1) return originalAvailableMode_Collabo024 (a1);
    const int status = TestMode::ReadTestModeValue (L"ModModeCollabo024");
    if (status == 1 && GetUserStatus () == 1) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo024 (a1);
}
HOOK (i64, AvailableMode_Collabo025, ASLR (0x1402DE6B0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo025 was called");
    const int tournamentMode = TestMode::ReadTestModeValue (L"TournamentMode");
    if (tournamentMode == 1) return originalAvailableMode_Collabo025 (a1);
    const int status = TestMode::ReadTestModeValue (L"ModModeCollabo025");
    if (status == 1 && GetUserStatus () == 1) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo025 (a1);
}
HOOK (i64, AvailableMode_Collabo026, ASLR (0x1402DE670), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo026 was called");
    const int tournamentMode = TestMode::ReadTestModeValue (L"TournamentMode");
    if (tournamentMode == 1) return originalAvailableMode_Collabo026 (a1);
    const int status = TestMode::ReadTestModeValue (L"ModModeCollabo026");
    if (status == 1 && GetUserStatus () == 1) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo026 (a1);
}
HOOK (i64, AvailableMode_AprilFool001, ASLR (0x1402DE5B0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_AprilFool001 was called");
    const int tournamentMode = TestMode::ReadTestModeValue (L"TournamentMode");
    if (tournamentMode == 1) return originalAvailableMode_AprilFool001 (a1);
    const int status = TestMode::ReadTestModeValue (L"ModModeAprilFool001");
    if (status == 1) return lua_pushbool (a1, true);
    return originalAvailableMode_AprilFool001 (a1);
}
i64 __fastcall lua_freeze_timer (const i64 a1) {
    LogMessage (LogLevel::HOOKS, "lua_freeze_timer was called");
    const int tournamentMode = TestMode::ReadTestModeValue (L"TournamentMode");
    if (tournamentMode == 1) return lua_pushbool (a1, true);
    const int status = TestMode::ReadTestModeValue (L"ModFreezeTimer");
    if (status == 1) return lua_pushbool (a1, true);
    return lua_pushbool (a1, false);
}
MID_HOOK (FreezeTimer, ASLR (0x14019FF51), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "FreezeTimer was called");
    const auto a1 = ctx.rdi;
    const int v9  = static_cast<int> (ctx.rax + 1);
    lua_pushcclosure (a1, reinterpret_cast<i64> (&lua_freeze_timer), v9);
    ctx.rip = ASLR (0x14019FF65);
}

void
ExecuteSendResultData () {
    luaL_dostring(lua_State, R"(
    local currentGameMode = PlayDataManager.GetPlayMode()

    if currentGameMode == GameMode.kEnso then
		if g_entryType == 0 then
            NetAccess.SendResultData(0)
        elseif g_entryType == 1 then
            NetAccess.SendResultData(1)
        else
            NetAccess.SendResultData(0)
            NetAccess.SendResultData(1)
        end
	elseif currentGameMode == GameMode.kAI then
		if g_joinSide_ == 1 then
            NetAccess.SendAiResultData(0)
        elseif g_joinSide_ == 2 then
            NetAccess.SendAiResultData(1)
        end
	elseif currentGameMode == GameMode.kCollabo025 then
		if g_joinSide_ == 1 then
            NetAccess.SendCollabo025AiResultData(0)
        elseif g_joinSide_ == 2 then
            NetAccess.SendCollabo025AiResultData(1)
        end
	elseif currentGameMode == GameMode.kCollabo026 then
		if g_joinSide_ == 1 then
            NetAccess.SendCollabo026AiResultData(0)
        elseif g_joinSide_ == 2 then
            NetAccess.SendCollabo026AiResultData(1)
        end
	elseif currentGameMode == GameMode.kAprilFool001 then
		if g_entryType == 0 then
            NetAccess.SendAprilFoolResultData(0)
        elseif g_entryType == 1 then
            NetAccess.SendAprilFoolResultData(1)
        else
            NetAccess.SendAprilFoolResultData(0)
            NetAccess.SendAprilFoolResultData(1)
        end
    end

    QR.StartQRExecAll(SceneType.kResult)
    )"
    );
}

SCENE_RESULT_HOOK (SceneResultInitialize_Enso, ASLR (0x140411FD0));
SCENE_RESULT_HOOK (SceneResultInitialize_AI, ASLR (0x140411FD0));
SCENE_RESULT_HOOK (SceneResultInitialize_Collabo025, ASLR (0x140411FD0));
SCENE_RESULT_HOOK (SceneResultInitialize_Collabo026, ASLR (0x140411FD0));
SCENE_RESULT_HOOK (SceneResultInitialize_AprilFool, ASLR (0x140411FD0));
SEND_RESULT_HOOK (SendResultData_Enso, ASLR (0x1401817B0));
SEND_RESULT_HOOK (SendResultData_AI, ASLR (0x1401755E0));
SEND_RESULT_HOOK (SendResultData_Collabo025_026, ASLR (0x140179A00));
SEND_RESULT_HOOK (SendResultData_AprilFool, ASLR (0x140177800));

CHANGE_RESULT_SIZE_HOOK (ChangeResultDataSize_Enso, ASLR (0x140180074), rax);
CHANGE_RESULT_SIZE_HOOK (ChangeResultDataSize_AI, ASLR (0x140173774), rax);
CHANGE_RESULT_SIZE_HOOK (ChangeResultDataSize_Collabo025_026, ASLR (0x140178244), rax);
CHANGE_RESULT_SIZE_HOOK (ChangeResultDataSize_AprilFool, ASLR (0x140176044), rax);

CHANGE_RESULT_INDEX_HOOK (ChangeResultDataIndex_Enso, ASLR (0x14018074B), rax, 0x34, 0x07);
CHANGE_RESULT_INDEX_HOOK (ChangeResultDataIndex_AI, ASLR (0x140173EDD), r13, 0x24, 0x08);
CHANGE_RESULT_INDEX_HOOK (ChangeResultDataIndex_Collabo025_026, ASLR (0x1401789AD), r13, 0x24, 0x08);
CHANGE_RESULT_INDEX_HOOK (ChangeResultDataIndex_AprilFool, ASLR (0x140176716), rax, 0x34, 0x06);

HOOK (i64, GetLanguage, ASLR (0x140024AC0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "GetLanguage was called");
    const auto result = originalGetLanguage (a1);
    language          = *reinterpret_cast<u32 *> (result);
    return result;
}
HOOK (i64, GetRegionLanguage, ASLR (0x1401CE9B0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "GetRegionLanguage was called");
    lua_settop (a1, 0);
    lua_pushstring (a1, languageStr (language));
    return 1;
}
HOOK (i64, GetCabinetLanguage, ASLR (0x1401D1A60), i64, i64 a2) {
    LogMessage (LogLevel::HOOKS, "GetCabinetLanguage was called");
    lua_settop (a2, 0);
    lua_pushstring (a2, languageStr (language));
    return 1;
}

MID_HOOK (ChangeLanguageType, ASLR (0x1400B2016), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "ChangeLanguageType was called");
    if (const auto pFontType = reinterpret_cast<int *> (ctx.rax); *pFontType == 4) *pFontType = 2;
}

MID_HOOK (CountLockedCrown, ASLR (0x1403F2A25), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "CountLockedCrown was called");
    ctx.r15 |= 1;
}

std::map<std::string, int> nus3bankMap;
int nus3bankIdCounter = 0;
std::map<std::string, bool> voiceCnExist;
bool enableSwitchVoice = false;
std::mutex nus3bankMtx;

int
get_bank_id (const std::string &bankName) {
    if (!nus3bankMap.contains (bankName)) {
        nus3bankMap[bankName] = nus3bankIdCounter;
        nus3bankIdCounter++;
    }
    return nus3bankMap[bankName];
}

void
check_voice_tail (const std::string &bankName, u8 *pBinfBlock, std::map<std::string, bool> &voiceExist, const std::string &tail) {
    // check if any voice_xxx.nus3bank has xxx_cn audio inside while loading
    if (enableSwitchVoice && bankName.starts_with ("voice_")) {
        const int binfLength = *reinterpret_cast<int *> (pBinfBlock + 4);
        u8 *pGrpBlock        = pBinfBlock + 8 + binfLength;
        const int grpLength  = *reinterpret_cast<int *> (pGrpBlock + 4);
        u8 *pDtonBlock       = pGrpBlock + 8 + grpLength;
        const int dtonLength = *reinterpret_cast<int *> (pDtonBlock + 4);
        u8 *pToneBlock       = pDtonBlock + 8 + dtonLength;
        const int toneSize   = *reinterpret_cast<int *> (pToneBlock + 8);
        u8 *pToneBase        = pToneBlock + 12;
        for (int i = 0; i < toneSize; i++) {
            if (*reinterpret_cast<int *> (pToneBase + i * 8 + 4) <= 0x0C) continue; // skip empty space
            u8 *currToneBase = pToneBase + *reinterpret_cast<int *> (pToneBase + i * 8);
            int titleOffset  = -1;
            switch (*currToneBase) {
            case 0xFF: titleOffset = 9; break; // audio mark
            case 0x7F: titleOffset = 5; break; // randomizer mark
            default: continue;                 // unknown mark skip
            }
            if (titleOffset > 0) {
                std::string title (reinterpret_cast<char *> (currToneBase + titleOffset));
                if (title.ends_with (tail)) {
                    if (!voiceExist.contains (bankName) || !voiceExist[bankName]) voiceExist[bankName] = true;
                    return;
                }
            }
        }
        if (!voiceExist.contains (bankName) || voiceExist[bankName]) voiceExist[bankName] = false;
    }
}

MID_HOOK (GenNus3bankId, ASLR (0x1407B97BD), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "GenNus3bankId was called");
    std::lock_guard lock (nus3bankMtx);
    if (reinterpret_cast<u8 **> (ctx.rcx + 8) != nullptr) {
        u8 *pNus3bankFile = *reinterpret_cast<u8 **> (ctx.rcx + 8);
        if (pNus3bankFile[0] == 'N' && pNus3bankFile[1] == 'U' && pNus3bankFile[2] == 'S' && pNus3bankFile[3] == '3') {
            const int tocLength  = *reinterpret_cast<int *> (pNus3bankFile + 16);
            u8 *pPropBlock       = pNus3bankFile + 20 + tocLength;
            const int propLength = *reinterpret_cast<int *> (pPropBlock + 4);
            u8 *pBinfBlock       = pPropBlock + 8 + propLength;
            const std::string bankName (reinterpret_cast<char *> (pBinfBlock + 0x11));
            check_voice_tail (bankName, pBinfBlock, voiceCnExist, "_cn");
            ctx.rax = get_bank_id (bankName);
        }
    }
}

std::string
FixToneName (const std::string &bankName, std::string toneName) {
    if (language == 2 || language == 4) {
        if (voiceCnExist.contains (bankName) && voiceCnExist[bankName]) return toneName + "_cn";
    }
    return toneName;
}

size_t commonSize = 0;
HOOK (i64, PlaySound, ASLR (0x1404C6DC0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "PlaySound was called");
    if (enableSwitchVoice && language != 0) {
        const std::string bankName (lua_tolstring (a1, -3, &commonSize));
        if (bankName[0] == 'v') {
            lua_pushstring (a1, FixToneName (bankName, lua_tolstring (a1, -2, &commonSize)).c_str ());
            lua_replace (a1, -3);
        }
    }
    return originalPlaySound (a1);
}

HOOK (i64, PlaySoundMulti, ASLR (0x1404C6D60), i64 a1) {
    LogMessage (LogLevel::HOOKS, "PlaySoundMulti was called");
    if (enableSwitchVoice && language != 0) {
        const std::string bankName (const_cast<char *> (lua_tolstring (a1, -3, &commonSize)));
        if (bankName[0] == 'v') {
            lua_pushstring (a1, FixToneName (bankName, lua_tolstring (a1, -2, &commonSize)).c_str ());
            lua_replace (a1, -3);
        }
    }
    return originalPlaySoundMulti (a1);
}

FUNCTION_PTR (u64 *, append_chars_to_basic_string, ASLR (0x140028DA0), u64 *, const char *, size_t);

u64 *
FixToneNameEnso (u64 *Src, const std::string &bankName) {
    if (language == 2 || language == 4) {
        if (voiceCnExist.contains (bankName) && voiceCnExist[bankName]) Src = append_chars_to_basic_string (Src, "_cn", 3);
    }
    return Src;
}

HOOK (bool, PlaySoundEnso, ASLR (0x1404ED590), u64 *a1, u64 *a2, i64 a3) {
    LogMessage (LogLevel::HOOKS, "PlaySoundEnso was called");
    if (enableSwitchVoice && language != 0) {
        const std::string bankName = a1[3] > 0x10 ? std::string (*reinterpret_cast<char **> (a1)) : std::string (reinterpret_cast<char *> (a1));
        if (bankName[0] == 'v') a2 = FixToneNameEnso (a2, bankName);
    }
    return originalPlaySoundEnso (a1, a2, a3);
}

HOOK (bool, PlaySoundSpecial, ASLR (0x1404ED230), u64 *a1, u64 *a2) {
    LogMessage (LogLevel::HOOKS, "PlaySoundSpecial was called");
    if (enableSwitchVoice && language != 0) {
        const std::string bankName = a1[3] > 0x10 ? std::string (*reinterpret_cast<char **> (a1)) : std::string (reinterpret_cast<char *> (a1));
        if (bankName[0] == 'v') a2 = FixToneNameEnso (a2, bankName);
    }
    return originalPlaySoundSpecial (a1, a2);
}

int loaded_fail_count = 0;
HOOK (i64, LoadedBankAll, ASLR (0x1404C69F0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "LoadedBankAll was called");
    originalLoadedBankAll (a1);
    const auto result = lua_toboolean (a1, -1);
    lua_settop (a1, 0);
    if (result) {
        loaded_fail_count = 0;
        lua_pushboolean (a1, 1);
    } else if (loaded_fail_count > 100) {
        loaded_fail_count = 0;
        lua_pushboolean (a1, 1);
    } else {
        loaded_fail_count += 1;
        lua_pushboolean (a1, 0);
    }
    return 1;
}

float soundRate = 1.0F;
HOOK (i32, SetMasterVolumeSpeaker, ASLR (0x140160330), i32 a1) {
    LogMessage (LogLevel::HOOKS, "SetMasterVolumeSpeaker was called");
    soundRate = static_cast<float> (a1 <= 100 ? 1.0F : a1 / 100.0);
    return originalSetMasterVolumeSpeaker (a1 > 100 ? 100 : a1);
}

HOOK (u64, NuscBusVolume, ASLR (0x1407B1C30), u64 a1, u64 a2, float a3) {
    LogMessage (LogLevel::HOOKS, "NuscBusVolume was called");
    return originalNuscBusVolume (a1, a2, a3 * soundRate);
}

std::string *fontName = nullptr;
HOOK (u8, SetupFontInfo, ASLR (0x14049D820), u64 a1, u64 a2, size_t a3, u64 a4) {
    LogMessage (LogLevel::HOOKS, "SetupFontInfo was called");
    if (fontName != nullptr) delete fontName;
    fontName = new std::string (reinterpret_cast<char *> (a1) + 120);
    return originalSetupFontInfo (a1, a2, a3, a4);
}

HOOK (u32, ReadFontInfoInt, ASLR (0x14049EAC0), u64 a1, u64 a2) {
    LogMessage (LogLevel::HOOKS, "ReadFontInfoInt was called");
    const std::string attribute (reinterpret_cast<char *> (a2));
    u32 result = originalReadFontInfoInt (a1, a2);
    if (fontName->starts_with ("cn_") && attribute == "offsetV") result += 1;
    return result;
}

constexpr i32 datatableBufferSize = 1024 * 1024 * 12;
safetyhook::Allocation datatableBuffer1;
safetyhook::Allocation datatableBuffer2;
safetyhook::Allocation datatableBuffer3;
const std::vector<uintptr_t> datatableBuffer1Addresses = {0x1400ABE40, 0x1400ABEB1, 0x1400ABEDB, 0x1400ABF4C};
const std::vector<uintptr_t> datatableBuffer2Addresses = {0x1400ABE2C, 0x1400ABF5B, 0x1400ABF8E};
const std::vector<uintptr_t> datatableBuffer3Addresses = {0x1400ABF7F, 0x1400ABF95, 0x1400ABFBF};
const std::vector<uintptr_t> memsetSizeAddresses       = {0x1400ABE26, 0x1400ABE3A, 0x1400ABF79};

void
AllocateStaticBufferNear (void *target_address, const size_t size, safetyhook::Allocation *newBuffer) {
    const auto allocator                = safetyhook::Allocator::global ();
    const std::vector desired_addresses = {static_cast<u8 *> (target_address)};
    auto allocation_result              = allocator->allocate_near (desired_addresses, size);
    if (allocation_result.has_value ()) *newBuffer = std::move (*allocation_result);
}

void
ReplaceLeaBufferAddress (const std::vector<uintptr_t> &bufferAddresses, void *newBufferAddress) {
    for (const auto bufferAddress : bufferAddresses) {
        const uintptr_t lea_instruction_dst = ASLR (bufferAddress) + 3;
        const uintptr_t lea_instruction_end = ASLR (bufferAddress) + 7;
        const intptr_t offset               = reinterpret_cast<intptr_t> (newBufferAddress) - lea_instruction_end;
        WRITE_MEMORY (lea_instruction_dst, i32, static_cast<i32> (offset));
    }
}

void
Init () {
    LogMessage (LogLevel::INFO, "Init JPN39 patches");
    i32 xRes          = 1920;
    i32 yRes          = 1080;
    bool vsync        = false;
    bool unlockSongs  = true;
    bool fixLanguage  = false;
    bool chsPatch     = false;
    bool useLayeredfs = false;

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        if (auto patches = openConfigSection (config_ptr.get (), "patches")) {
            unlockSongs = readConfigBool (patches, "unlock_songs", unlockSongs);
            if (auto jpn39 = openConfigSection (patches, "jpn39")) {
                fixLanguage = readConfigBool (jpn39, "fix_language", fixLanguage);
                chsPatch    = readConfigBool (jpn39, "chs_patch", chsPatch);
            }
        }

        if (auto graphics = openConfigSection (config_ptr.get (), "graphics")) {
            if (auto res = openConfigSection (graphics, "res")) {
                xRes = static_cast<i32> (readConfigInt (res, "x", xRes));
                yRes = static_cast<i32> (readConfigInt (res, "y", yRes));
            }
            vsync = readConfigBool (graphics, "vsync", vsync);
        }

        if (auto layeredfs = openConfigSection (config_ptr.get (), "layeredfs")) useLayeredfs = readConfigBool (layeredfs, "enabled", useLayeredfs);
    }

    // Hook to get AppAccessor and ComponentAccessor
    INSTALL_HOOK (DeviceCheck);
    INSTALL_HOOK (luaL_newstate);

    // Apply common config patch
    WRITE_MEMORY (ASLR (0x140494533), i32, xRes);
    WRITE_MEMORY (ASLR (0x14049453A), i32, yRes);
    if (!vsync) WRITE_MEMORY (ASLR (0x14064C7E9), u8, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90);
    if (unlockSongs) {
        WRITE_MEMORY (ASLR (0x1403F45CF), u8, 0xB0, 0x01);
        INSTALL_MID_HOOK (CountLockedCrown);
    }

    // Bypass errors
    WRITE_MEMORY (ASLR (0x140041A00), u8, 0xC3);

    // Use TLS v1.2
    WRITE_MEMORY (ASLR (0x140580459), u8, 0x10);

    // Disable F/G check
    WRITE_MEMORY (ASLR (0x140CD4858), char, "./");

    // Move various files to current directory
    WRITE_MEMORY (ASLR (0x140C8AB60), char, "./"); // F:/
    WRITE_MEMORY (ASLR (0x140C8AB5C), char, "./"); // G:/
    WRITE_MEMORY (ASLR (0x140CD4DC0), char, ".\\Setting1.bin");
    WRITE_MEMORY (ASLR (0x140CD4DB0), char, ".\\Setting2.bin");
    WRITE_MEMORY (ASLR (0x140C95B40), char, ".\\TournamentData\\PlayData\\TournamentPlayData.dat");
    WRITE_MEMORY (ASLR (0x140C95B78), char, ".\\TournamentData\\PlayData\\TournamentPlayData.dat");
    WRITE_MEMORY (ASLR (0x140C95BB0), char, ".\\TournamentData\\InfoData\\TournamentInfoData.dat");
    WRITE_MEMORY (ASLR (0x140C95BE8), char, ".\\TournamentData\\InfoData\\TournamentInfoData.dat");
    WRITE_MEMORY (ASLR (0x140CC0508), char, ".\\Garmc\\BillingData\\GarmcBillingData.dat");
    WRITE_MEMORY (ASLR (0x140CC0538), char, ".\\Garmc\\BillingData\\GarmcBillingData.dat");
    WRITE_MEMORY (ASLR (0x140CC0660), char, ".\\Garmc\\BillingData\\GarmcOBillingData.dat");
    WRITE_MEMORY (ASLR (0x140CC0690), char, ".\\Garmc\\BillingData\\GarmcOBillingData.dat");
    WRITE_MEMORY (ASLR (0x140CC05E0), char, ".\\Garmc\\BillingNetIdLocationId\\GarmcBillingNetIdLocationId.dat");
    WRITE_MEMORY (ASLR (0x140CC0620), char, ".\\Garmc\\BillingNetIdLocationId\\GarmcBillingNetIdLocationId.dat");
    WRITE_MEMORY (ASLR (0x140CC0830), char, ".\\Garmc\\BillingNetIdLocationId\\GarmcOBillingNetIdLocationId.dat");
    WRITE_MEMORY (ASLR (0x140CC0880), char, ".\\Garmc\\BillingNetIdLocationId\\GarmcOBillingNetIdLocationId.dat");
    WRITE_MEMORY (ASLR (0x140CC06C0), char, ".\\Garmc\\ErrorLogData\\GarmcErrorLogData.dat");
    WRITE_MEMORY (ASLR (0x140CC06F0), char, ".\\Garmc\\ErrorLogData\\GarmcErrorLogData.dat");
    WRITE_MEMORY (ASLR (0x140CC0580), char, ".\\Garmc\\ErrorLogData\\GarmcOErrorLogData.dat");
    WRITE_MEMORY (ASLR (0x140CC05B0), char, ".\\Garmc\\ErrorLogData\\GarmcOErrorLogData.dat");

    // Remove datatable size limit
    {
        for (auto address : memsetSizeAddresses)
            WRITE_MEMORY (ASLR (address) + 2, i32, datatableBufferSize);

        auto bufferBase = MODULE_HANDLE - 0x03000000;
        AllocateStaticBufferNear (bufferBase, datatableBufferSize, &datatableBuffer1);
        bufferBase += datatableBufferSize;
        AllocateStaticBufferNear (bufferBase, datatableBufferSize, &datatableBuffer2);
        bufferBase += datatableBufferSize;
        AllocateStaticBufferNear (bufferBase, datatableBufferSize, &datatableBuffer3);

        ReplaceLeaBufferAddress (datatableBuffer1Addresses, datatableBuffer1.data ());
        ReplaceLeaBufferAddress (datatableBuffer2Addresses, datatableBuffer2.data ());
        ReplaceLeaBufferAddress (datatableBuffer3Addresses, datatableBuffer3.data ());
    }

    // Freeze Timer
    TestMode::RegisterItem (L"<select-item label=\"FREEZE TIMER\" param-offset-x=\"35\" replace-text=\"0:OFF, 1:ON\" "
                            L"group=\"Setting\" id=\"ModFreezeTimer\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] { INSTALL_MID_HOOK (FreezeTimer); });
    // Mode Unlock
    TestMode::RegisterItem (L"<select-item label=\"KIMETSU MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, 1:ENABLE, "
                            L"2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo024\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] { INSTALL_HOOK (AvailableMode_Collabo024); });
    TestMode::RegisterItem (L"<select-item label=\"ONE PIECE MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, "
                            L"1:ENABLE, 2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo025\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] { INSTALL_HOOK (AvailableMode_Collabo025); });
    TestMode::RegisterItem (L"<select-item label=\"AI SOSHINA MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, "
                            L"1:ENABLE, 2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo026\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] { INSTALL_HOOK (AvailableMode_Collabo026); });
    TestMode::RegisterItem (L"<select-item label=\"AOHARU MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, 1:ENABLE, "
                            L"2:CARD ONLY\" group=\"Setting\" id=\"ModModeAprilFool001\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] { INSTALL_HOOK (AvailableMode_AprilFool001); });
    TestMode::RegisterItem (L"<select-item label=\"INSTANT RESULT\" param-offset-x=\"35\" replace-text=\"0:OFF, 1:ON\" "
                            L"group=\"Setting\" id=\"ModInstantResult\" max=\"1\" min=\"0\" default=\"0\"/>",
                            [&] {
                                INSTALL_HOOK (SceneResultInitialize_Enso);
                                INSTALL_HOOK (SceneResultInitialize_AI);
                                INSTALL_HOOK (SceneResultInitialize_Collabo025);
                                INSTALL_HOOK (SceneResultInitialize_Collabo026);
                                INSTALL_HOOK (SceneResultInitialize_AprilFool);
                                INSTALL_HOOK (SendResultData_Enso);
                                INSTALL_HOOK (SendResultData_AI);
                                INSTALL_HOOK (SendResultData_Collabo025_026);
                                INSTALL_HOOK (SendResultData_AprilFool);
                                INSTALL_MID_HOOK (ChangeResultDataSize_Enso);
                                INSTALL_MID_HOOK (ChangeResultDataSize_AI);
                                INSTALL_MID_HOOK (ChangeResultDataSize_Collabo025_026);
                                INSTALL_MID_HOOK (ChangeResultDataSize_AprilFool);
                                INSTALL_MID_HOOK (ChangeResultDataIndex_Enso);
                                INSTALL_MID_HOOK (ChangeResultDataIndex_AI);
                                INSTALL_MID_HOOK (ChangeResultDataIndex_Collabo025_026);
                                INSTALL_MID_HOOK (ChangeResultDataIndex_AprilFool);
                            });
    // Unlimit Volume
    TestMode::RegisterModify (
        L"/root/menu[@id='SoundTestMenu']/layout[@type='Center']/select-item[@id='OutputLevelSpeakerItem']",
        [&] (const pugi::xml_node &node) {
            TestMode::Append (node, L"label", L"*");
            node.attribute (L"max").set_value (L"300");
            node.attribute (L"delta").set_value (L"1");
        },
        [&] () {
            INSTALL_HOOK (SetMasterVolumeSpeaker);
            INSTALL_HOOK (NuscBusVolume);
        });
    // Instant Result
    // TestMode::RegisterModify(
    //     L"/root/menu[@id='GameOptionsMenu']/layout[@type='Center']/select-item[@id='NumberOfStageItem']",
    //     [&](pugi::xml_node &node) { TestMode::Append(node, L"label", L"*"); node.attribute(L"max").set_value(L"99"); },
    //     [&](){}
    // );

    // Use chs font/wordlist instead of cht
    if (chsPatch) {
        bool fontExistAll = true;
        const char *fontToCheck[]{"cn_30.nutexb", "cn_30.xml", "cn_32.nutexb", "cn_32.xml", "cn_64.nutexb", "cn_64.xml"};
        for (int i = 0; i < 6; i++) {
            if (useLayeredfs && std::filesystem::exists (std::string ("..\\..\\Data_mods\\x64\\font\\") + fontToCheck[i])) continue;
            if (std::filesystem::exists (std::string ("..\\..\\Data\\x64\\font\\") + fontToCheck[i])) continue;
            fontExistAll = false;
        }

        if (fontExistAll) {
            WRITE_MEMORY (ASLR (0x140CD1AE0), char, "cn_64");
            WRITE_MEMORY (ASLR (0x140CD1AF0), char, "cn_32");
            WRITE_MEMORY (ASLR (0x140CD1AF8), char, "cn_30");
            WRITE_MEMORY (ASLR (0x140C946A0), char, "chineseSText");
            WRITE_MEMORY (ASLR (0x140C946B0), char, "chineseSFontType");
            WRITE_MEMORY (ASLR (0x140CD1E40), wchar_t, L"加載中\0");
            WRITE_MEMORY (ASLR (0x140CD1E28), wchar_t, L"加載中.\0");
            WRITE_MEMORY (ASLR (0x140CD1E68), wchar_t, L"加載中..\0");
            WRITE_MEMORY (ASLR (0x140CD1E50), wchar_t, L"加載中...\0");
            INSTALL_MID_HOOK (ChangeLanguageType);
            INSTALL_HOOK (SetupFontInfo);
            INSTALL_HOOK (ReadFontInfoInt);
        }

        LayeredFs::RegisterBefore ([=] (const std::string &originalFileName, const std::string &currentFileName) -> std::string {
            if (currentFileName.find ("\\lumen\\") == std::string::npos) return "";
            std::string fileName = currentFileName;
            fileName             = replace (fileName, "\\lumen\\", "\\lumen_cn\\");
            if (std::filesystem::exists (fileName)) return fileName;
            return currentFileName;
        });
    }

    // Fix language
    if (fixLanguage) {
        INSTALL_HOOK (GetLanguage); // Language will use in other place
        INSTALL_HOOK (GetRegionLanguage);
        INSTALL_HOOK (GetCabinetLanguage);
    }

    // if both chsPatch & fixLanguage enabled, try use cn voice from nus3bank
    // don't worry, we will check if cn voice existed
    if (chsPatch && fixLanguage) {
        enableSwitchVoice = true;
        INSTALL_HOOK (PlaySound);
        INSTALL_HOOK (PlaySoundMulti);
        INSTALL_HOOK (PlaySoundEnso);
        INSTALL_HOOK (PlaySoundSpecial);
    }

    // Fix normal song play after passing through silent song
    INSTALL_MID_HOOK (GenNus3bankId);
    INSTALL_HOOK (LoadedBankAll);

    // Disable live check
    auto amHandle = reinterpret_cast<u64> (GetModuleHandle ("AMFrameWork.dll"));
    INSTALL_HOOK_DYNAMIC (AMFWTerminate, reinterpret_cast<void *> (amHandle + 0x42DE0));

    // Move various files to current directory
    WRITE_MEMORY (amHandle + 0x15252, u8, 0xEB); // CreditLogPathA
    WRITE_MEMORY (amHandle + 0x15419, u8, 0xEB); // CreditLogPathB
    WRITE_MEMORY (amHandle + 0x416DA, u8, 0xEB); // ErrorLogPathA
    WRITE_MEMORY (amHandle + 0x41859, u8, 0xEB); // ErrorLogPathB
    WRITE_MEMORY (amHandle + 0x41C21, u8, 0xEB); // CommonLogPathA
    WRITE_MEMORY (amHandle + 0x41DA5, u8, 0xEB); // CommonLogPathB
    WRITE_NOP (amHandle + 0x420F1, 0x05);        // BackupDataPathA
    WRITE_NOP (amHandle + 0x42167, 0x05);        // BackupDataPathB

    // Redirect garmc requests
    auto garmcHandle = reinterpret_cast<u64> (GetModuleHandle ("garmc.dll"));
    INSTALL_HOOK_DYNAMIC (curl_easy_setopt, reinterpret_cast<void *> (garmcHandle + 0x1FBBB0));
}
} // namespace patches::JPN39
