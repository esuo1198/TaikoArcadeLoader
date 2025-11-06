// ReSharper disable CppTooWideScopeInitStatement
#include "helpers.h"
#include "../patches.h"
#include <map>
#include "config.h"

extern u64 song_data_size;
extern void *song_data;

#define RDX_MOV 0x48, 0xBA
#define R8_MOV  0x49, 0xB8
#define GENERATE_MOV(instruction, location)                                                                                 \
    instruction, (u8)(u64)(location), (u8)((u64)(location) >> 8), (u8)((u64)(location) >> 16), (u8)((u64)(location) >> 24), \
        (u8)((u64)(location) >> 32), (u8)((u64)(location) >> 40), (u8)((u64)(location) >> 48), (u8)((u64)(location) >> 56)

static const i32& xRes = Config::ConfigManager::instance ().getGraphicsConfig ().res.x;
static const i32& yRes = Config::ConfigManager::instance ().getGraphicsConfig ().res.y;
static bool vsync = Config::ConfigManager::instance ().getGraphicsConfig ().vsync;
static bool localFiles = Config::ConfigManager::instance ().getPatchesConfig ().local_files;

namespace patches::JPN39 {
int language = 0;
FAST_HOOK_DYNAMIC (char, AMFWTerminate, i64) {
    LogMessage (LogLevel::HOOKS, "AMFWTerminate was called");
    return 0;
}

FAST_HOOK_DYNAMIC (i64, curl_easy_setopt, i64 a1, i64 a2, i64 a3, i64 a4, i64 a5) {
    LogMessage (LogLevel::HOOKS, "Garmc curl_easy_setopt was called");
    originalcurl_easy_setopt.call<i64> (a1, 64, 0, 0, 0);
    originalcurl_easy_setopt.call<i64> (a1, 81, 0, 0, 0);
    return originalcurl_easy_setopt.call<i64> (a1, a2, a3, a4, a5);
}
FAST_HOOK_DYNAMIC (void, garmc_logger_log, i64 a1, int a2, void *a3, char a4) {
    // remove garmc log
    return;
}

FUNCTION_PTR (i64, GetPlayDataManagerRef, ASLR (0x140024AC0), i64);

FUNCTION_PTR (u64, RefTestModeMain, ASLR (0x1400337C0), u64);
FUNCTION_PTR (u64, RefPlayDataManager, ASLR (0x140024AC0), u64);
FUNCTION_PTR (i64, GetUserCount, ASLR (0x1403F1080), u64);

i64
lua_pushbool (Lua::LuaState a1, const bool val) {
    Lua::SetTop (a1, 0);
    Lua::PushBoolean (a1, val);
    return 1;
}

u64 appAccessor       = 0;
u64 componentAccessor = 0;
FAST_HOOK (i64, DeviceCheck, ASLR (0x140464FC0), i64 a1, i64 a2, i64 a3) {
    LogMessage (LogLevel::HOOKS, "DeviceCheck was called");
    TestMode::SetupAccessor (a3, RefTestModeMain);
    componentAccessor = a2;
    appAccessor = a3;
    return originalDeviceCheck.fastcall<i64> (a1, a2, a3);
}

i64
GetPlayererCount () {
    if (appAccessor) {
        if (const u64 playDataManager = RefPlayDataManager (appAccessor)) return GetUserCount (playDataManager);
    }
    return -1;
}
boolean
HaveCardPlayer () {
    if (appAccessor) {
        if (const u64 playDataManager = RefPlayDataManager (appAccessor)) {
            int p1PlayerType = *(i32 *)(*(i64 *)(playDataManager + 9920) + 4);
            int p2PlayerType = *(i32 *)(*(i64 *)(playDataManager + 9928) + 4);
            return p1PlayerType == 2 || p2PlayerType == 2;
        }
    }
    return false;
}

TestMode::Value *unlockSongs = TestMode::CreateValue (L"ModUnlockSongs");
FAST_HOOK (bool, IsSongRelease, ASLR (0x1403F4510), i64 a1, i64 a2, int a3) {
    if (unlockSongs->Read () == 1) return true;
    return originalIsSongRelease.fastcall<bool> (a1, a2, a3);
}
FAST_HOOK (bool, IsSongReleasePlayer, ASLR (0x1403F45F0), i64 a1, u64 a2, i32 a3) {
    if (unlockSongs->Read () == 2) return true;
    return originalIsSongReleasePlayer.fastcall<bool> (a1, a2, a3);
}
MID_HOOK (DifficultyPanelCrown, ASLR (0x1403F2A25), SafetyHookContext &ctx) {
    if (unlockSongs->Read () != 1) return;
    ctx.r15 |= 1;
}
TestMode::Value *tournmentMode = TestMode::CreateValue (L"TournamentMode");
TestMode::Value *mode024 = TestMode::CreateValue (L"ModModeCollabo024");
TestMode::Value *mode025 = TestMode::CreateValue (L"ModModeCollabo025");
TestMode::Value *mode026 = TestMode::CreateValue (L"ModModeCollabo026");
TestMode::Value *modeApril001 = TestMode::CreateValue (L"ModModeAprilFool001");
FAST_HOOK (i64, AvailableMode_Collabo024, ASLR (0x1402DE710), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo024 was called");
    const int tournamentMode = tournmentMode->Read ();
    if (tournamentMode == 1) return originalAvailableMode_Collabo024.fastcall<i64> (a1);
    const int status = mode024->Read ();
    if (status == 1 && GetPlayererCount () == 1) return lua_pushbool (a1, true);
    else if (status == 2 && GetPlayererCount () == 1 && HaveCardPlayer ()) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo024.fastcall<i64> (a1);
}
FAST_HOOK (i64, AvailableMode_Collabo025, ASLR (0x1402DE6B0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo025 was called");
    const int tournamentMode = tournmentMode->Read ();
    if (tournamentMode == 1) return originalAvailableMode_Collabo025.fastcall<i64> (a1);
    const int status = mode025->Read ();
    if (status == 1 && GetPlayererCount () == 1) return lua_pushbool (a1, true);
    else if (status == 2 && GetPlayererCount () == 1 && HaveCardPlayer ()) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo025.fastcall<i64> (a1);
}
FAST_HOOK (i64, AvailableMode_Collabo026, ASLR (0x1402DE670), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_Collabo026 was called");
    const int tournamentMode = tournmentMode->Read ();
    if (tournamentMode == 1) return originalAvailableMode_Collabo026.fastcall<i64> (a1);
    const int status = mode026->Read ();
    if (status == 1 && GetPlayererCount () == 1) return lua_pushbool (a1, true);
    else if (status == 2 && GetPlayererCount () == 1 && HaveCardPlayer ()) return lua_pushbool (a1, true);
    return originalAvailableMode_Collabo026.fastcall<i64> (a1);
}
FAST_HOOK (i64, AvailableMode_AprilFool001, ASLR (0x1402DE5B0), i64 a1) {
    LogMessage (LogLevel::HOOKS, "AvailableMode_AprilFool001 was called");
    const int tournamentMode = tournmentMode->Read ();
    if (tournamentMode == 1) return originalAvailableMode_AprilFool001.fastcall<i64> (a1);
    const int status = modeApril001->Read ();
    if (status == 1) return lua_pushbool (a1, true);
    else if (status == 2 && HaveCardPlayer ()) return lua_pushbool (a1, true);
    return originalAvailableMode_AprilFool001.fastcall<i64> (a1);
}
TestMode::Value *freezeTimer = TestMode::CreateValue (L"ModFreezeTimer");
int lua_freeze_timer (Lua::LuaState a1) {
    LogMessage (LogLevel::HOOKS, "lua_freeze_timer was called");
    const int tournamentMode = tournmentMode->Read ();
    if (tournamentMode == 1) return (int)lua_pushbool (a1, true);
    const int status = freezeTimer-> Read ();
    if (status == 1) return (int)lua_pushbool (a1, true);
    return (int)lua_pushbool (a1, false);
}
MID_HOOK (FreezeTimer, ASLR (0x14019FF51), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "FreezeTimer was called");
    const auto a1 = ctx.rdi;
    const int v9  = static_cast<int> (ctx.rax + 1);
    Lua::PushCClosure (a1, lua_freeze_timer, v9);
    ctx.rip = ASLR (0x14019FF65);
}

void
ExecuteSendResultData () {
    Lua::Execute(R"(
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

bool sendFlag = false;
TestMode::Value *instantResult = TestMode::CreateValue (L"ModInstantResult");
TestMode::Value *stageNumber = TestMode::CreateValue (L"NumberOfStageItem");
#define SCENE_RESULT_HOOK(functionName, location)                       \
    FAST_HOOK (void, functionName, location, i64 a1, i64 a2, i64 a3) {  \
        if (instantResult->Read () != 1 && stageNumber->Read () <= 4) { \
            original##functionName.fastcall (a1, a2, a3);               \
            return;                                                     \
        }                                                               \
        sendFlag = true;                                                \
        original##functionName.fastcall (a1, a2, a3);                   \
        ExecuteSendResultData ();                                       \
    }

#define SEND_RESULT_HOOK(functionName, location)                        \
    FAST_HOOK (void, functionName, location, i64 a1) {                  \
        if (instantResult->Read () != 1 && stageNumber->Read () <= 4) { \
            original##functionName.fastcall (a1);                       \
            return;                                                     \
        }                                                               \
        if (sendFlag) {                                                 \
            sendFlag = false;                                           \
            original##functionName.fastcall (a1);                       \
        }                                                               \
    }

#define CHANGE_RESULT_SIZE_HOOK(functionName, location, target)                   \
    MID_HOOK (functionName, location, SafetyHookContext &ctx) {                   \
        if (instantResult->Read () != 1 && stageNumber->Read () <= 4) { return; } \
        i64 instance          = RefPlayDataManager (*(i64 *)ctx.r12);             \
        u32 currentStageCount = *(u32 *)(instance + 8);                           \
        ctx.target &= 0xFFFFFFFF00000000;                                         \
        ctx.target |= currentStageCount;                                          \
    }

#define CHANGE_RESULT_INDEX_HOOK(functionName, location, target, offset, skip)    \
    MID_HOOK (functionName, location, SafetyHookContext &ctx) {                   \
        if (instantResult->Read () != 1 && stageNumber->Read () <= 4) { return; } \
        i64 instance          = RefPlayDataManager (*(i64 *)ctx.r12);             \
        u32 currentStageCount = *(u32 *)(instance + 8);                           \
        ctx.target &= 0xFFFFFFFF00000000;                                         \
        ctx.target |= currentStageCount - 1;                                      \
        *(u32 *)(ctx.rsp + offset) = currentStageCount - 1;                       \
        ctx.rip += skip;                                                          \
    }

SCENE_RESULT_HOOK (SceneResultInitialize_Enso, ASLR (0x140411FD0));
SCENE_RESULT_HOOK (SceneResultInitialize_AI, ASLR (0x14041E8C0));
SCENE_RESULT_HOOK (SceneResultInitialize_Collabo025, ASLR (0x140434060));
SCENE_RESULT_HOOK (SceneResultInitialize_Collabo026, ASLR (0x140438BF0));
SCENE_RESULT_HOOK (SceneResultInitialize_AprilFool, ASLR (0x140429A80));
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

MID_HOOK (CountLockedCrown, ASLR (0x1403F2A25), SafetyHookContext &ctx) {
    LogMessage (LogLevel::HOOKS, "CountLockedCrown was called");
    ctx.r15 |= 1;
}

int loaded_fail_count = 0;
FAST_HOOK (char, LoadedBankAll, ASLR (0x1404EC2F0), u64 a1) {
    LogMessage (LogLevel::HOOKS, "LoadedBankAll was called");
    char result = originalLoadedBankAll.fastcall<char> (a1);
    if (result) {
        loaded_fail_count = 0;
        return 1;
    } else if (loaded_fail_count > 60) {
        LogMessage (LogLevel::WARN, "LoadBankAll Guardian!!!");
        loaded_fail_count = 0;
        return 1;
    } else {
        loaded_fail_count += 1;
        return 0;
    }
}

float soundRate = 1.0F;
FAST_HOOK (i32, SetMasterVolumeSpeaker, ASLR (0x140160330), i32 a1) {
    LogMessage (LogLevel::HOOKS, "SetMasterVolumeSpeaker was called");
    patches::Audio::SetVolumeRate (a1 <= 100 ? 1.0f : a1 / 100.0f);
    return originalSetMasterVolumeSpeaker.fastcall<i32> (a1 > 100 ? 100 : a1);
}

TestMode::Value *attractDemo = TestMode::CreateValue (L"AttractDemoItem");
MID_HOOK (AttractDemo, ASLR (0x14045A720), SafetyHookContext &ctx) {
    if (attractDemo->Read () == 1) ctx.r14 = 0;
}

FAST_HOOK (DWORD*, AcquireMostCompatibleDisplayMode, ASLR (0x14064C870), i64 a1, DWORD *a2, DWORD *a3) {
    LogMessage (LogLevel::DEBUG, "AcquireMostCompatibleDisplayMode {:d} {:d} {:f} {:f}", a3[0], a3[1], (float)(int)a3[2], (float)(int)a3[3]);
    a3[2] = (DWORD)(float)120.0;
    // a3[0] = xRes;
    // a3[1] = yRes;
    LogMessage (LogLevel::DEBUG, "AcquireMostCompatibleDisplayMode {:d} {:d} {:f} {:f}", a3[0], a3[1], (float)(int)a3[2], (float)(int)a3[3]);
    return originalAcquireMostCompatibleDisplayMode.fastcall<DWORD *> (a1, a2, a3);
}

constexpr i32 datatableBufferSize = 1024 * 1024 * 12;
uint8_t *datatableBuffer[3] = { nullptr };
std::vector<SafetyHookMid> datatable_patch = {};
#define DATATABLE_PATCH_REGISTER(location, reg, value, skip) { datatable_patch.push_back(safetyhook::create_mid(location, [](SafetyHookContext &ctx) {ctx.reg = (uintptr_t)(value); ctx.rip = location + skip;})); }
void
PatchDatatable () {
    LogMessage (LogLevel::INFO, "Using Datatable patch size: 12MB");
    for (int i = 0; i < 3; i ++) datatableBuffer[i] = (uint8_t *)malloc (datatableBufferSize);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABE26), r8, datatableBufferSize, 6);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABE3A), r8, datatableBufferSize, 6);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF79), r8, datatableBufferSize, 6);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABE40), rcx, datatableBuffer[0], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABEB1), rdi, datatableBuffer[0], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABEDB), rdi, datatableBuffer[0], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF4C), rdi, datatableBuffer[0], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABE2C), rcx, datatableBuffer[1], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF5B), rcx, datatableBuffer[1], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF8E), r8 , datatableBuffer[1], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF7F), rcx, datatableBuffer[2], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABF95), rcx, datatableBuffer[2], 7);
    DATATABLE_PATCH_REGISTER (ASLR (0x1400ABFBF), rdx, datatableBuffer[2], 7);
}

void
Init () {
    LogMessage (LogLevel::DEBUG, "Init JPN39 patches");
    bool unlockSongs  = Config::ConfigManager::instance ().getPatchesConfig ().unlock_songs;
    double modelResRate = Config::ConfigManager::instance ().getGraphicsConfig ().model_res_rate;

    // Hook to get AppAccessor and ComponentAccessor
    INSTALL_FAST_HOOK (DeviceCheck);
    // INSTALL_FAST_HOOK (luaL_newstate);
    // INSTALL_FAST_HOOK (AcquireMostCompatibleDisplayMode);

    // Window Size
    WRITE_MEMORY (ASLR (0x140494533), i32, xRes);
    WRITE_MEMORY (ASLR (0x14049453A), i32, yRes);
    // DonSystem::DonModelRenderer::DonModelRenderer
    if (modelResRate > 0) {
        i32 donModelX = (i32)(xRes * modelResRate);
        i32 donModelY = (i32)(yRes * modelResRate);
        LogMessage (LogLevel::INFO, "Using DonModel Resolutin: {}x{}", donModelX, donModelY);
        WRITE_MEMORY (ASLR (0x1404F3D5B), i32, donModelX);
        WRITE_MEMORY (ASLR (0x1404F3D62), i32, donModelY);
    }

    if (!vsync) WRITE_MEMORY (ASLR (0x14064C7E9), u8, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90);

    // Bypass errors
    WRITE_MEMORY (ASLR (0x140041A00), u8, 0xC3);

    // Use TLS v1.2
    WRITE_MEMORY (ASLR (0x140580459), u8, 0x10);

    // Remove datatable size limit
    PatchDatatable ();

    // Unlock Songs
    TestMode::RegisterItem (
        L"<select-item label=\"UNLOCK SONGS\" param-offset-x=\"35\" replace-text=\"0:OFF, 1:ON, "
        L"2:FORCE\" group=\"Setting\" id=\"ModUnlockSongs\" max=\"2\" min=\"0\" default=\"1\"/>",
        [&]() {
            INSTALL_FAST_HOOK (IsSongRelease);
            INSTALL_FAST_HOOK (IsSongReleasePlayer);
            INSTALL_MID_HOOK (DifficultyPanelCrown);
        }
    );
    // Freeze Timer
    TestMode::RegisterItem (
        L"<select-item label=\"FREEZE TIMER\" param-offset-x=\"35\" replace-text=\"0:OFF, 1:ON\" "
        L"group=\"Setting\" id=\"ModFreezeTimer\" max=\"1\" min=\"0\" default=\"0\"/>",
        [&] { INSTALL_MID_HOOK (FreezeTimer); }
    );
    // Mode Unlock
    // TestMode::Menu *modeUnlock = TestMode::CreateMenu (L"MODE UNLOCK", L"ModeUnlockMenu");
    TestMode::RegisterItem (
        L"<select-item label=\"KIMETSU MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, 1:ENABLE, "
        L"2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo024\" max=\"2\" min=\"0\" default=\"0\"/>",
        [&] { INSTALL_FAST_HOOK (AvailableMode_Collabo024); }//, modeUnlock
    );
    TestMode::RegisterItem (
        L"<select-item label=\"ONE PIECE MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, "
        L"1:ENABLE, 2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo025\" max=\"2\" min=\"0\" default=\"0\"/>",
        [&] { INSTALL_FAST_HOOK (AvailableMode_Collabo025); }//, modeUnlock
    );
    TestMode::RegisterItem (
        L"<select-item label=\"AI SOSHINA MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, "
        L"1:ENABLE, 2:CARD ONLY\" group=\"Setting\" id=\"ModModeCollabo026\" max=\"2\" min=\"0\" default=\"0\"/>",
        [&] { INSTALL_FAST_HOOK (AvailableMode_Collabo026); }//, modeUnlock
    );
    TestMode::RegisterItem (
        L"<select-item label=\"AOHARU MODE\" param-offset-x=\"35\" replace-text=\"0:DEFAULT, 1:ENABLE, "
        L"2:CARD ONLY\" group=\"Setting\" id=\"ModModeAprilFool001\" max=\"2\" min=\"0\" default=\"0\"/>",
        [&] { INSTALL_FAST_HOOK (AvailableMode_AprilFool001); }//, modeUnlock
    );
    // TestMode::RegisterItem (modeUnlock);
    // InstantResult
    TestMode::RegisterItem (
        L"<select-item label=\"INSTANT RESULT\" param-offset-x=\"35\" replace-text=\"0:OFF, 1:ON\" "
        L"group=\"Setting\" id=\"ModInstantResult\" max=\"1\" min=\"0\" default=\"0\"/>",
        [&] {
            INSTALL_FAST_HOOK (SceneResultInitialize_Enso);
            INSTALL_FAST_HOOK (SceneResultInitialize_AI);
            INSTALL_FAST_HOOK (SceneResultInitialize_Collabo025);
            INSTALL_FAST_HOOK (SceneResultInitialize_Collabo026);
            INSTALL_FAST_HOOK (SceneResultInitialize_AprilFool);
            INSTALL_FAST_HOOK (SendResultData_Enso);
            INSTALL_FAST_HOOK (SendResultData_AI);
            INSTALL_FAST_HOOK (SendResultData_Collabo025_026);
            INSTALL_FAST_HOOK (SendResultData_AprilFool);
            INSTALL_MID_HOOK (ChangeResultDataSize_Enso);
            INSTALL_MID_HOOK (ChangeResultDataSize_AI);
            INSTALL_MID_HOOK (ChangeResultDataSize_Collabo025_026);
            INSTALL_MID_HOOK (ChangeResultDataSize_AprilFool);
            INSTALL_MID_HOOK (ChangeResultDataIndex_Enso);
            INSTALL_MID_HOOK (ChangeResultDataIndex_AI);
            INSTALL_MID_HOOK (ChangeResultDataIndex_Collabo025_026);
            INSTALL_MID_HOOK (ChangeResultDataIndex_AprilFool);
        }
    );
    // Unlimit Volume
    TestMode::RegisterModify (
        L"/root/menu[@id='SoundTestMenu']/layout[@type='Center']/select-item[@id='OutputLevelSpeakerItem']",
        [&] (const pugi::xml_node &node) {
            TestMode::Append (node, L"label", L"*");
            node.attribute (L"max").set_value (L"300");
            node.attribute (L"delta").set_value (L"1");
        },
        [&] () { INSTALL_FAST_HOOK (SetMasterVolumeSpeaker); }
    );
    TestMode::RegisterItemAfter(
        L"/root/menu[@id='OthersMenu']/layout[@type='Center']/select-item[@id='AttractMovieItem']",
        L"<select-item label=\"ATTRACT DEMO\" disable=\"True/LanguageItem:0\" param-offset-x=\"35\" "
        L"replace-text=\"0:ON, 1:OFF\" group=\"Setting\" id=\"AttractDemoItem\" max=\"1\" min=\"0\" default=\"0\"/>",
        [&](){ INSTALL_MID_HOOK (AttractDemo); }
    );

    // Instant Result
    // TestMode::RegisterModify(
    //     L"/root/menu[@id='GameOptionsMenu']/layout[@type='Center']/select-item[@id='NumberOfStageItem']",
    //     [&](pugi::xml_node &node) { TestMode::Append(node, L"label", L"*"); node.attribute(L"max").set_value(L"99"); },
    //     [&](){}
    // );

    // Fix normal song play after passing through silent song
    // INSTALL_MID_HOOK (GenNus3bankId);
    INSTALL_FAST_HOOK (LoadedBankAll);

    // Disable live check
    auto amHandle = reinterpret_cast<u64> (GetModuleHandle ("AMFrameWork.dll"));
    INSTALL_FAST_HOOK_DYNAMIC (AMFWTerminate, reinterpret_cast<void *> (amHandle + 0x42DE0));

    if (localFiles){
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

        {
            // Song_id limit (1599)
            WRITE_MEMORY (ASLR (0x1402e0d5d), i32, 9000); // EBP,0x63f
            WRITE_MEMORY (ASLR (0x1403f1a86), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f1e14), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f1f24), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f2044), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f2184), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f33f4), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403fd936), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403fda00), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403fdb34), i32, 9000); // EDI,0x63f
            WRITE_MEMORY (ASLR (0x1403f1b46), i32, 9000); // ESI,0x63f
            WRITE_MEMORY (ASLR (0x1404050eb), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x1404051b8), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x140405285), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x1404053ab), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x140405478), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x140405545), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x14040564a), i32, 9000); // RBX,0x63f
            WRITE_MEMORY (ASLR (0x1403f3c3a), i32, 9000); // dword ptr [RBX],0x63f

            // Song_id limit (1600)
            WRITE_MEMORY (ASLR (0x1402e1ab6), i32, 9000); // EAX,0x640
            WRITE_MEMORY (ASLR (0x1403f452f), i32, 9000); // EBP,0x640
            WRITE_MEMORY (ASLR (0x1403f2365), i32, 9000); // EBP,0x640
            WRITE_MEMORY (ASLR (0x1403f4609), i32, 9000); // EBX,0x640
            WRITE_MEMORY (ASLR (0x1403f4c02), i32, 9000); // EBX,0x640
            WRITE_MEMORY (ASLR (0x140454c6c), i32, 9000); // EBX,0x640
            WRITE_MEMORY (ASLR (0x1403fdc03), i32, 9000); // EDI,0x640
            WRITE_MEMORY (ASLR (0x14014e0ad), i32, 9000); // ESI,0x640
            WRITE_MEMORY (ASLR (0x1403f24ec), i32, 9000); // ESI,0x640
            WRITE_MEMORY (ASLR (0x1403f30a1), i32, 9000); // ESI,0x640
            WRITE_MEMORY (ASLR (0x14043e6b2), i32, 9000); // ESI,0x640
            WRITE_MEMORY (ASLR (0x14043e6fe), i32, 9000); // ESI,0x640
            WRITE_MEMORY (ASLR (0x14015935d), i32, 9000); // param_1,0x640
            WRITE_MEMORY (ASLR (0x14015ccd1), i32, 9000); // param_1,0x640
            WRITE_MEMORY (ASLR (0x14015cd01), i32, 9000); // param_1,0x640
            WRITE_MEMORY (ASLR (0x1403e0fb5), i32, 9000); // param_1,0x640
            WRITE_MEMORY (ASLR (0x1403e139f), i32, 9000); // param_1,0x640
            WRITE_MEMORY (ASLR (0x1403f4686), i32, 9000); // param_3,0x640
            WRITE_MEMORY (ASLR (0x1403f46d7), i32, 9000); // param_3,0x640
            WRITE_MEMORY (ASLR (0x1403f4cc6), i32, 9000); // param_3,0x640
            WRITE_MEMORY (ASLR (0x1404056ee), i32, 9000); // param_3,0x640
            WRITE_MEMORY (ASLR (0x1402e1c66), i32, 9000); // R14D,0x640
            WRITE_MEMORY (ASLR (0x1402e22b6), i32, 9000); // R14D,0x640
            WRITE_MEMORY (ASLR (0x1403f29fb), i32, 9000); // R14D,0x640
            WRITE_MEMORY (ASLR (0x1403f2a91), i32, 9000); // R14D,0x640
            // WRITE_MEMORY (ASLR (0x14003cd30), i32, 9000); // R15D,0x640

            song_data = malloc (song_data_size);
            memset (song_data, 0, song_data_size);

            // Ranks, Crowns, etc
            WRITE_MEMORY (ASLR (0x1403F1A9B), u8, GENERATE_MOV (R8_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403FDA16), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403FD94C), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403FDB4A), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403F3407), u8, GENERATE_MOV (R8_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403F2062), u8, GENERATE_MOV (R8_MOV, song_data));
            WRITE_MEMORY (ASLR (0x14040555C), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1404051CF), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x140405102), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x14040529C), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1404051CF), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x140405102), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x14040548F), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x140405661), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1404053C2), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_NOP (ASLR (0x14040566B), 0x05);
            WRITE_MEMORY (ASLR (0x1403F1B75), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403F1E2B), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403F1F6C), u8, GENERATE_MOV (RDX_MOV, song_data));
            WRITE_MEMORY (ASLR (0x1403F21CC), u8, GENERATE_MOV (RDX_MOV, song_data));
        }

        // Move various files to current directory
        WRITE_MEMORY (amHandle + 0x15252, u8, 0xEB); // CreditLogPathA
        WRITE_MEMORY (amHandle + 0x15419, u8, 0xEB); // CreditLogPathB
        WRITE_MEMORY (amHandle + 0x416DA, u8, 0xEB); // ErrorLogPathA
        WRITE_MEMORY (amHandle + 0x41859, u8, 0xEB); // ErrorLogPathB
        WRITE_MEMORY (amHandle + 0x41C21, u8, 0xEB); // CommonLogPathA
        WRITE_MEMORY (amHandle + 0x41DA5, u8, 0xEB); // CommonLogPathB
        WRITE_NOP (amHandle + 0x420F1, 0x05);        // BackupDataPathA
        WRITE_NOP (amHandle + 0x42167, 0x05);        // BackupDataPathB
    }

    // Redirect garmc requests
    auto garmcHandle = reinterpret_cast<u64> (GetModuleHandle ("garmc.dll"));
    INSTALL_FAST_HOOK_DYNAMIC (curl_easy_setopt, reinterpret_cast<void *> (garmcHandle + 0x1FBBB0));
    INSTALL_FAST_HOOK_DYNAMIC (garmc_logger_log, reinterpret_cast<void *> (garmcHandle + 0x13AB70));
}
} // namespace patches::JPN39
