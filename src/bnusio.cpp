#include "constants.h"
#include "helpers.h"
#include "patches/patches.h"
#include "poll.h"

extern GameVersion version;
extern std::vector<HMODULE> plugins;
extern u64 song_data_size;
extern void *song_data;
extern char accessCode1[21];
extern char accessCode2[21];
extern char chipId1[33];
extern char chipId2[33];
extern bool autoIme;
extern bool emulateUsio;
extern bool emulateCardReader;

typedef i32 (*callbackAttach) (i32, i32, i32 *);
typedef void (*callbackTouch) (i32, i32, u8[168], u64);
typedef void event ();
typedef void waitTouchEvent (callbackTouch, u64);
bool waitingForTouch = false;
callbackTouch touchCallback;
u64 touchData;
callbackAttach attachCallback;
i32 *attachData;

Keybindings EXIT          = {.keycodes = {VK_ESCAPE}};
Keybindings TEST          = {.keycodes = {VK_F1}};
Keybindings SERVICE       = {.keycodes = {VK_F2}};
Keybindings DEBUG_UP      = {.keycodes = {VK_UP}};
Keybindings DEBUG_DOWN    = {.keycodes = {VK_DOWN}};
Keybindings DEBUG_ENTER   = {.keycodes = {VK_RETURN}};
Keybindings COIN_ADD      = {.keycodes = {VK_RETURN}, .buttons = {SDL_CONTROLLER_BUTTON_START}};
Keybindings CARD_INSERT_1 = {.keycodes = {'P'}};
Keybindings CARD_INSERT_2 = {};
Keybindings QR_DATA_READ  = {.keycodes = {'Q'}};
Keybindings QR_IMAGE_READ = {.keycodes = {'W'}};
Keybindings P1_LEFT_BLUE  = {.keycodes = {'D'}, .axis = {SDL_AXIS_LEFT_DOWN}};
Keybindings P1_LEFT_RED   = {.keycodes = {'F'}, .axis = {SDL_AXIS_LEFT_RIGHT}};
Keybindings P1_RIGHT_RED  = {.keycodes = {'J'}, .axis = {SDL_AXIS_RIGHT_RIGHT}};
Keybindings P1_RIGHT_BLUE = {.keycodes = {'K'}, .axis = {SDL_AXIS_RIGHT_DOWN}};
Keybindings P2_LEFT_BLUE  = {};
Keybindings P2_LEFT_RED   = {};
Keybindings P2_RIGHT_RED  = {};
Keybindings P2_RIGHT_BLUE = {};

bool testEnabled  = false;
int coin_count    = 0;
bool inited       = false;
HWND windowHandle = nullptr;
HKL currentLayout;

namespace bnusio {
#define RETURN_FALSE(returnType, functionName, ...) \
    returnType functionName (__VA_ARGS__) { return 0; }

extern "C" {
RETURN_FALSE (i64, bnusio_Open);
RETURN_FALSE (i64, bnusio_Close);
RETURN_FALSE (i64, bnusio_Communication, i32 a1);
RETURN_FALSE (u8, bnusio_IsConnected);
RETURN_FALSE (i32, bnusio_ResetIoBoard);
RETURN_FALSE (u16, bnusio_GetStatusU16, u16 a1);
RETURN_FALSE (u8, bnusio_GetStatusU8, u16 a1);
RETURN_FALSE (u16, bnusio_GetRegisterU16, i16 a1);
RETURN_FALSE (u8, bnusio_GetRegisterU8, u16 a1);
RETURN_FALSE (void *, bnusio_GetBuffer, u16 a1, i64 a2, i16 a3);
RETURN_FALSE (i64, bnusio_SetRegisterU16, u16 a1, u16 a2);
RETURN_FALSE (i64, bnusio_SetRegisterU8, u16 a1, u8 a2);
RETURN_FALSE (i64, bnusio_SetBuffer, u16 a1, i32 a2, i16 a3);
RETURN_FALSE (void *, bnusio_GetSystemError);
RETURN_FALSE (i64, bnusio_SetSystemError, i16 a1);
RETURN_FALSE (i64, bnusio_ClearSram);
RETURN_FALSE (void *, bnusio_GetExpansionMode);
RETURN_FALSE (i64, bnusio_SetExpansionMode, i16 a1);
RETURN_FALSE (u8, bnusio_IsWideUsio);
RETURN_FALSE (u64, bnusio_GetSwIn64);
RETURN_FALSE (u8, bnusio_GetGout, u8 a1);
RETURN_FALSE (i64, bnusio_SetGout, u8 a1, u8 a2);
RETURN_FALSE (u64, bnusio_GetEncoder);
RETURN_FALSE (i64, bnusio_GetCoinLock, u8 a1);
RETURN_FALSE (i64, bnusio_SetCoinLock, u8 a1, u8 a2);
RETURN_FALSE (i64, bnusio_GetCDOut, u8 a1);
RETURN_FALSE (i64, bnusio_SetCDOut, u8 a1, u8 a2);
RETURN_FALSE (i64, bnusio_GetHopOut, u8 a1);
RETURN_FALSE (i64, bnusio_SetHopOut, u8 a1, u8 a2);
RETURN_FALSE (void *, bnusio_SetPLCounter, i16 a1);
RETURN_FALSE (char *, bnusio_GetIoBoardName);
RETURN_FALSE (i64, bnusio_SetHopperRequest, u16 a1, i16 a2);
RETURN_FALSE (i64, bnusio_SetHopperLimit, u16 a1, i16 a2);
RETURN_FALSE (i64, bnusio_SramRead, i32 a1, u8 a2, i32 a3, u16 a4);
RETURN_FALSE (i64, bnusio_SramWrite, i32 a1, u8 a2, i32 a3, u16 a4);
RETURN_FALSE (void *, bnusio_GetCoinError, i32 a1);
RETURN_FALSE (void *, bnusio_GetService, i32 a1);
RETURN_FALSE (void *, bnusio_GetServiceError, i32 a1);
RETURN_FALSE (i64, bnusio_DecCoin, i32 a1, u16 a2);
RETURN_FALSE (i64, bnusio_DecService, i32 a1, u16 a2);
RETURN_FALSE (i64, bnusio_ResetCoin);
size_t
bnusio_GetFirmwareVersion () {
    return 126;
}

u32
bnusio_GetSwIn () {
    u32 sw = 0;
    sw |= (u32)testEnabled << 7;
    sw |= (u32)IsButtonDown (DEBUG_ENTER) << 9;
    sw |= (u32)IsButtonDown (DEBUG_DOWN) << 12;
    sw |= (u32)IsButtonDown (DEBUG_UP) << 13;
    sw |= (u32)IsButtonDown (SERVICE) << 14;
    return sw;
}

u16 drumWaitPeriod = 4;
bool valueStates[] = {false, false, false, false, false, false, false, false};

Keybindings *analogButtons[]
    = {&P1_LEFT_BLUE, &P1_LEFT_RED, &P1_RIGHT_RED, &P1_RIGHT_BLUE, &P2_LEFT_BLUE, &P2_LEFT_RED, &P2_RIGHT_RED, &P2_RIGHT_BLUE};

u16 buttonWaitPeriodP1 = 0;
u16 buttonWaitPeriodP2 = 0;
std::queue<u8> buttonQueueP1;
std::queue<u8> buttonQueueP2;

bool analogInput;
SDLAxis analogBindings[] = {
    SDL_AXIS_LEFT_LEFT,  SDL_AXIS_LEFT_RIGHT,  SDL_AXIS_LEFT_DOWN,  SDL_AXIS_LEFT_UP,  // P1: LB, LR, RR, RB
    SDL_AXIS_RIGHT_LEFT, SDL_AXIS_RIGHT_RIGHT, SDL_AXIS_RIGHT_DOWN, SDL_AXIS_RIGHT_UP, // P2: LB, LR, RR, RB
};

u16
bnusio_GetAnalogIn (u8 which) {
    u16 analogValue;
    if (analogInput) {
        analogValue = (u16)(32768 * ControllerAxisIsDown (analogBindings[which]));
        if (analogValue > 100) return analogValue;
        return 0;
    }
    auto button = analogButtons[which];
    if (which == 0) {
        if (buttonWaitPeriodP1 > 0) buttonWaitPeriodP1--;
        if (buttonWaitPeriodP2 > 0) buttonWaitPeriodP2--;
    }
    bool isP1 = which / 4 == 0;
    if ((isP1 && !buttonQueueP1.empty ()) || (!isP1 && !buttonQueueP2.empty ())) {
        if ((isP1 && buttonQueueP1.front () == which && buttonWaitPeriodP1 == 0)
            || (!isP1 && buttonQueueP2.front () == which && buttonWaitPeriodP2 == 0)) {
            if (isP1) {
                buttonQueueP1.pop ();
                buttonWaitPeriodP1 = drumWaitPeriod;
            } else {
                buttonQueueP2.pop ();
                buttonWaitPeriodP2 = drumWaitPeriod;
            }

            u16 hitValue       = !valueStates[which] ? 50 : 51;
            valueStates[which] = !valueStates[which];
            return (hitValue << 15) / 100 + 1;
        }
        if (IsButtonTapped (*button)) {
            if (isP1) buttonQueueP1.push (which);
            else buttonQueueP2.push (which);
        }
        return 0;
    } else if (IsButtonTapped (*button)) {
        if (isP1 && buttonWaitPeriodP1 > 0) {
            buttonQueueP1.push (which);
            return 0;
        } else if (!isP1 && buttonWaitPeriodP2 > 0) {
            buttonQueueP2.push (which);
            return 0;
        }
        if (isP1) buttonWaitPeriodP1 = drumWaitPeriod;
        else buttonWaitPeriodP2 = drumWaitPeriod;

        u16 hitValue       = !valueStates[which] ? 50 : 51;
        valueStates[which] = !valueStates[which];
        return (hitValue << 15) / 100 + 1;
    } else {
        return 0;
    }
}

u16 __fastcall bnusio_GetCoin (i32 a1) { return coin_count; }
}

FUNCTION_PTR (i64, bnusio_Open_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_Open"));
FUNCTION_PTR (i64, bnusio_Close_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_Close"));
FUNCTION_PTR (u64, bnusio_Communication_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_Communication"), i32);
FUNCTION_PTR (u8, bnusio_IsConnected_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_IsConnected"));
FUNCTION_PTR (i32, bnusio_ResetIoBoard_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_ResetIoBoard"));
FUNCTION_PTR (u16, bnusio_GetStatusU16_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetStatusU16"), u16);
FUNCTION_PTR (u8, bnusio_GetStatusU8_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetStatusU8"), u16);
FUNCTION_PTR (u16, bnusio_GetRegisterU16_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetRegisterU16"), i16);
FUNCTION_PTR (u8, bnusio_GetRegisterU8_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetRegisterU8"), i16);
FUNCTION_PTR (void *, bnusio_GetBuffer_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetBuffer"), u16, i64, i16);
FUNCTION_PTR (i64, bnusio_SetRegisterU16_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetRegisterU16"), u16, u16);
FUNCTION_PTR (i64, bnusio_SetRegisterU8_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetRegisterU8"), u16, u8);
FUNCTION_PTR (i64, bnusio_SetBuffer_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetBuffer"), u16, i32, i16);
FUNCTION_PTR (void *, bnusio_GetSystemError_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetSystemError"));
FUNCTION_PTR (i64, bnusio_SetSystemError_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetSystemError"), i16);
FUNCTION_PTR (size_t, bnusio_GetFirmwareVersion_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetFirmwareVersion"));
FUNCTION_PTR (i64, bnusio_ClearSram_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_ClearSram"));
FUNCTION_PTR (void *, bnusio_GetExpansionMode_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetExpansionMode"));
FUNCTION_PTR (i64, bnusio_SetExpansionMode_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetExpansionMode"), i16);
FUNCTION_PTR (u8, bnusio_IsWideUsio_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_IsWideUsio"));
FUNCTION_PTR (u32, bnusio_GetSwIn_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetSwIn"));
FUNCTION_PTR (u64, bnusio_GetSwIn64_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetSwIn64"));
FUNCTION_PTR (u16, bnusio_GetAnalogIn_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetAnalogIn"), u8);
FUNCTION_PTR (u8, bnusio_GetGout_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetGout"), u8);
FUNCTION_PTR (i64, bnusio_SetGout_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetGout"), u8, u8);
FUNCTION_PTR (u64, bnusio_GetEncoder_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetEncoder"));
FUNCTION_PTR (i64, bnusio_GetCoinLock_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetCoinLock"), u8);
FUNCTION_PTR (i64, bnusio_SetCoinLock_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetCoinLock"), u8, u8);
FUNCTION_PTR (i64, bnusio_GetCDOut_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetCDOut"), u8);
FUNCTION_PTR (i64, bnusio_SetCDOut_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetCDOut"), u8, u8);
FUNCTION_PTR (i64, bnusio_GetHopOut_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetHopOut"), u8);
FUNCTION_PTR (i64, bnusio_SetHopOut_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetHopOut"), u8, u8);
FUNCTION_PTR (void *, bnusio_SetPLCounter_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetPLCounter"), i16);
FUNCTION_PTR (char *, bnusio_GetIoBoardName_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetIoBoardName"));
FUNCTION_PTR (i64, bnusio_SetHopperRequest_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetHopperRequest"), u16, i16);
FUNCTION_PTR (i64, bnusio_SetHopperLimit_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SetHopperLimit"), u16, i16);
FUNCTION_PTR (i64, bnusio_SramRead_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SramRead"), i32, u8, i32, u16);
FUNCTION_PTR (i64, bnusio_SramWrite_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_SramWrite"), i32, u8, i32, u16);
FUNCTION_PTR (u16, bnusio_GetCoin_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetCoin"), i32);
FUNCTION_PTR (void *, bnusio_GetCoinError_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetCoinError"), i32);
FUNCTION_PTR (void *, bnusio_GetService_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetService"), i32);
FUNCTION_PTR (void *, bnusio_GetServiceError_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_GetServiceError"), i32);
FUNCTION_PTR (i64, bnusio_DecCoin_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_DecCoin"), i32, u16);
FUNCTION_PTR (u64, bnusio_DecService_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_DecService"), i32, u16);
FUNCTION_PTR (i64, bnusio_ResetCoin_Original, PROC_ADDRESS ("bnusio_original.dll", "bnusio_ResetCoin"));

HOOK (u64, bngrw_Init, PROC_ADDRESS ("bngrw.dll", "BngRwInit")) { return 0; }
HOOK (void, bngrw_Fin, PROC_ADDRESS ("bngrw.dll", "BngRwFin")) { return; }
HOOK (u64, bngrw_IsCmdExec, PROC_ADDRESS ("bngrw.dll", "BngRwIsCmdExec")) { return 0xFFFFFFFF; }
HOOK (i32, bngrw_ReqCancel, PROC_ADDRESS ("bngrw.dll", "BngRwReqCancel")) { return 1; }
HOOK (i32, bngrw_ReqSendUrl, PROC_ADDRESS ("bngrw.dll", "BngRwReqSendUrlTo")) { return 1; }
HOOK (u64, bngrw_ReqLed, PROC_ADDRESS ("bngrw.dll", "BngRwReqLed")) { return 1; }
HOOK (u64, bngrw_ReqBeep, PROC_ADDRESS ("bngrw.dll", "BngRwReqBeep")) { return 1; }
HOOK (u64, bngrw_ReqAction, PROC_ADDRESS ("bngrw.dll", "BngRwReqAction")) { return 1; }
HOOK (u64, bngrw_ReqSetLedPower, PROC_ADDRESS ("bngrw.dll", "BngRwReqSetLedPower")) { return 0; }
HOOK (u64, bngrw_GetRetryCount, PROC_ADDRESS ("bngrw.dll", "BngRwGetTotalRetryCount")) { return 0; }
HOOK (u64, bngrw_GetFwVersion, PROC_ADDRESS ("bngrw.dll", "BngRwGetFwVersion")) { return 0; }
HOOK (u64, bngrw_ReqFwVersionUp, PROC_ADDRESS ("bngrw.dll", "BngRwReqFwVersionUp")) { return 1; }
HOOK (u64, bngrw_ReqFwCleanup, PROC_ADDRESS ("bngrw.dll", "BngRwReqFwCleanup")) { return 1; }
HOOK (u64, bngrw_ReadMifare, PROC_ADDRESS ("bngrw.dll", "BngRwExReadMifareAllBlock")) { return 0xFFFFFF9C; }
HOOK (u64, bngrw_GetStationID, PROC_ADDRESS ("bngrw.dll", "BngRwGetStationID")) { return 0; }
HOOK (i32, bngrw_ReqSendMail, PROC_ADDRESS ("bngrw.dll", "BngRwReqSendMailTo")) { return 1; }
HOOK (i32, bngrw_ReqLatchID, PROC_ADDRESS ("bngrw.dll", "BngRwReqLatchID")) { return 1; }
HOOK (u64, bngrw_ReqAiccAuth, PROC_ADDRESS ("bngrw.dll", "BngRwReqAiccAuth")) { return 1; }
HOOK (u64, bngrw_DevReset, PROC_ADDRESS ("bngrw.dll", "BngRwDevReset")) { return 1; }
HOOK (u64, bngrw_Attach, PROC_ADDRESS ("bngrw.dll", "BngRwAttach"), i32 a1, char *a2, i32 a3, i32 a4, i32 (*callback) (i32, i32, i32 *), i32 *a6) {
    // This is way too fucking jank
    attachCallback = callback;
    attachData     = a6;
    return 1;
}
HOOK (u64, bngrw_ReqWaitTouch, PROC_ADDRESS ("bngrw.dll", "BngRwReqWaitTouch"), u32 a1, i32 a2, u32 a3, void (*callback) (i32, i32, u8[168], u64),
      u64 a5) {
    waitingForTouch = true;
    touchCallback   = callback;
    touchData       = a5;
    for (auto plugin : plugins) {
        FARPROC touchEvent = GetProcAddress (plugin, "WaitTouch");
        if (touchEvent) ((waitTouchEvent *)touchEvent) (callback, a5);
    }
    return 1;
}

void
Init () {
    SetKeyboardButtons ();

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        toml_table_t *config = config_ptr.get ();
        auto drum            = openConfigSection (config, "drum");
        if (drum) drumWaitPeriod = readConfigInt (drum, "wait_period", drumWaitPeriod);
        auto controller = openConfigSection (config, "controller");
        if (controller) {
            analogInput = readConfigBool (controller, "analog_input", analogInput);
            if (analogInput) printf ("Using analog input mode. All the keyboard drum inputs have been disabled.\n");
        }
    }

    auto keyconfigPath = std::filesystem::current_path () / "keyconfig.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> keyconfig_ptr (openConfig (keyconfigPath), toml_free);
    if (keyconfig_ptr) {
        toml_table_t *keyconfig = keyconfig_ptr.get ();
        SetConfigValue (keyconfig, "EXIT", &EXIT);

        SetConfigValue (keyconfig, "TEST", &TEST);
        SetConfigValue (keyconfig, "SERVICE", &SERVICE);
        SetConfigValue (keyconfig, "DEBUG_UP", &DEBUG_UP);
        SetConfigValue (keyconfig, "DEBUG_DOWN", &DEBUG_DOWN);
        SetConfigValue (keyconfig, "DEBUG_ENTER", &DEBUG_ENTER);

        SetConfigValue (keyconfig, "COIN_ADD", &COIN_ADD);
        SetConfigValue (keyconfig, "CARD_INSERT_1", &CARD_INSERT_1);
        SetConfigValue (keyconfig, "CARD_INSERT_2", &CARD_INSERT_2);
        SetConfigValue (keyconfig, "QR_DATA_READ", &QR_DATA_READ);
        SetConfigValue (keyconfig, "QR_IMAGE_READ", &QR_IMAGE_READ);

        SetConfigValue (keyconfig, "P1_LEFT_BLUE", &P1_LEFT_BLUE);
        SetConfigValue (keyconfig, "P1_LEFT_RED", &P1_LEFT_RED);
        SetConfigValue (keyconfig, "P1_RIGHT_RED", &P1_RIGHT_RED);
        SetConfigValue (keyconfig, "P1_RIGHT_BLUE", &P1_RIGHT_BLUE);
        SetConfigValue (keyconfig, "P2_LEFT_BLUE", &P2_LEFT_BLUE);
        SetConfigValue (keyconfig, "P2_LEFT_RED", &P2_LEFT_RED);
        SetConfigValue (keyconfig, "P2_RIGHT_RED", &P2_RIGHT_RED);
        SetConfigValue (keyconfig, "P2_RIGHT_BLUE", &P2_RIGHT_BLUE);
    }

    if (!emulateUsio && !std::filesystem::exists (std::filesystem::current_path () / "bnusio_original.dll")) {
        emulateUsio = true;
        std::cerr << "[Init] bnusio_original.dll not found! usio emulation enabled" << std::endl;
    }

    if (!emulateUsio) {
        INSTALL_HOOK_DIRECT (bnusio_Open, bnusio_Open_Original);
        INSTALL_HOOK_DIRECT (bnusio_Close, bnusio_Close_Original);
        INSTALL_HOOK_DIRECT (bnusio_Communication, bnusio_Communication_Original);
        INSTALL_HOOK_DIRECT (bnusio_IsConnected, bnusio_IsConnected_Original);
        INSTALL_HOOK_DIRECT (bnusio_ResetIoBoard, bnusio_ResetIoBoard_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetStatusU16, bnusio_GetStatusU16_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetStatusU8, bnusio_GetStatusU8_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetRegisterU16, bnusio_GetRegisterU16_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetRegisterU8, bnusio_GetRegisterU8_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetBuffer, bnusio_GetBuffer_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetRegisterU16, bnusio_SetRegisterU16_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetRegisterU8, bnusio_SetRegisterU8_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetBuffer, bnusio_SetBuffer_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetSystemError, bnusio_GetSystemError_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetSystemError, bnusio_SetSystemError_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetFirmwareVersion, bnusio_GetFirmwareVersion_Original);
        INSTALL_HOOK_DIRECT (bnusio_ClearSram, bnusio_ClearSram_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetExpansionMode, bnusio_GetExpansionMode_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetExpansionMode, bnusio_SetExpansionMode_Original);
        INSTALL_HOOK_DIRECT (bnusio_IsWideUsio, bnusio_IsWideUsio_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetSwIn, bnusio_GetSwIn_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetSwIn64, bnusio_GetSwIn64_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetAnalogIn, bnusio_GetAnalogIn_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetGout, bnusio_GetGout_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetGout, bnusio_SetGout_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetEncoder, bnusio_GetEncoder_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetCoinLock, bnusio_GetCoinLock_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetCoinLock, bnusio_SetCoinLock_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetCDOut, bnusio_GetCDOut_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetCDOut, bnusio_SetCDOut_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetHopOut, bnusio_GetHopOut_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetHopOut, bnusio_SetHopOut_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetPLCounter, bnusio_SetPLCounter_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetIoBoardName, bnusio_GetIoBoardName_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetHopperRequest, bnusio_SetHopperRequest_Original);
        INSTALL_HOOK_DIRECT (bnusio_SetHopperLimit, bnusio_SetHopperLimit_Original);
        INSTALL_HOOK_DIRECT (bnusio_SramRead, bnusio_SramRead_Original);
        INSTALL_HOOK_DIRECT (bnusio_SramWrite, bnusio_SramWrite_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetCoin, bnusio_GetCoin_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetCoinError, bnusio_GetCoinError_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetService, bnusio_GetService_Original);
        INSTALL_HOOK_DIRECT (bnusio_GetServiceError, bnusio_GetServiceError_Original);
        INSTALL_HOOK_DIRECT (bnusio_DecCoin, bnusio_DecCoin_Original);
        INSTALL_HOOK_DIRECT (bnusio_DecService, bnusio_DecService_Original);
        INSTALL_HOOK_DIRECT (bnusio_ResetCoin, bnusio_ResetCoin_Original);

        std::cout << "[Init] USIO emulation disabled" << std::endl;
    }

    if (emulateCardReader) {
        INSTALL_HOOK (bngrw_Init)
        INSTALL_HOOK (bngrw_Fin);
        INSTALL_HOOK (bngrw_IsCmdExec);
        INSTALL_HOOK (bngrw_ReqCancel);
        INSTALL_HOOK (bngrw_ReqWaitTouch);
        INSTALL_HOOK (bngrw_ReqSendUrl);
        INSTALL_HOOK (bngrw_ReqLed);
        INSTALL_HOOK (bngrw_ReqBeep);
        INSTALL_HOOK (bngrw_ReqAction);
        INSTALL_HOOK (bngrw_ReqSetLedPower);
        INSTALL_HOOK (bngrw_GetRetryCount);
        INSTALL_HOOK (bngrw_GetFwVersion);
        INSTALL_HOOK (bngrw_ReqFwVersionUp);
        INSTALL_HOOK (bngrw_ReqFwCleanup);
        INSTALL_HOOK (bngrw_ReadMifare);
        INSTALL_HOOK (bngrw_GetStationID);
        INSTALL_HOOK (bngrw_ReqSendMail);
        INSTALL_HOOK (bngrw_ReqLatchID);
        INSTALL_HOOK (bngrw_ReqAiccAuth);
        INSTALL_HOOK (bngrw_Attach);
        INSTALL_HOOK (bngrw_DevReset);
    } else {
        std::cout << "[Init] Card reader emulation disabled" << std::endl;
    }
}

void
Update () {
    if (!inited) {
        windowHandle = FindWindowA ("nuFoundation.Window", nullptr);
        InitializePoll (windowHandle);
        if (autoIme) {
            currentLayout  = GetKeyboardLayout (0);
            auto engLayout = LoadKeyboardLayout (TEXT ("00000409"), KLF_ACTIVATE);
            ActivateKeyboardLayout (engLayout, KLF_SETFORPROCESS);
        }

        for (auto plugin : plugins) {
            auto initEvent = GetProcAddress (plugin, "Init");
            if (initEvent) initEvent ();
        }

        inited = true;
    }

    UpdatePoll (windowHandle);
    if (IsButtonTapped (COIN_ADD) && !testEnabled) coin_count++;
    if (IsButtonTapped (TEST)) testEnabled = !testEnabled;
    if (IsButtonTapped (EXIT)) ExitProcess (0);
    if (waitingForTouch) {
        static u8 cardData[168]
            = {0x01, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x2E, 0x58, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x5C, 0x97, 0x44, 0xF0, 0x88, 0x04, 0x00, 0x43, 0x26, 0x2C, 0x33, 0x00, 0x04,
               0x06, 0x10, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
               0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30,
               0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x42, 0x47, 0x49, 0x43, 0x36,
               0x00, 0x00, 0xFA, 0xE9, 0x69, 0x00, 0xF6, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        bool hasInserted = false;
        if (IsButtonTapped (CARD_INSERT_1)) {
            for (auto plugin : plugins) {
                FARPROC insertEvent = GetProcAddress (plugin, "BeforeCard1Insert");
                if (insertEvent) ((event *)insertEvent) ();
            }
            for (auto plugin : plugins) {
                FARPROC insertEvent = GetProcAddress (plugin, "Card1Insert");
                if (insertEvent) {
                    ((event *)insertEvent) ();
                    hasInserted = true;
                }
            }
            if (!hasInserted) {
                memcpy (cardData + 0x2C, chipId1, 33);
                memcpy (cardData + 0x50, accessCode1, 21);
                touchCallback (0, 0, cardData, touchData);
            }
        } else if (IsButtonTapped (CARD_INSERT_2)) {
            for (auto plugin : plugins) {
                FARPROC insertEvent = GetProcAddress (plugin, "BeforeCard2Insert");
                if (insertEvent) ((event *)insertEvent) ();
            }
            for (auto plugin : plugins) {
                FARPROC insertEvent = GetProcAddress (plugin, "Card2Insert");
                if (insertEvent) {
                    ((event *)insertEvent) ();
                    hasInserted = true;
                }
            }
            if (!hasInserted) {
                memcpy (cardData + 0x2C, chipId2, 33);
                memcpy (cardData + 0x50, accessCode2, 21);
                touchCallback (0, 0, cardData, touchData);
            }
        }
    }

    for (auto plugin : plugins) {
        auto updateEvent = GetProcAddress (plugin, "Update");
        if (updateEvent) updateEvent ();
    }

    patches::Qr::Update ();

    if (attachCallback) attachCallback (0, 0, attachData);
}

void
Close () {
    if (autoIme) ActivateKeyboardLayout (currentLayout, KLF_SETFORPROCESS);
    for (auto plugin : plugins) {
        FARPROC exitEvent = GetProcAddress (plugin, "Exit");
        if (exitEvent) ((event *)exitEvent) ();
    }
}
} // namespace bnusio
