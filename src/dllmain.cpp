#include "bnusio.h"
#include "constants.h"
#include "helpers.h"
#include "patches/patches.h"
#include "poll.h"

GameVersion gameVersion = GameVersion::UNKNOWN;
std::vector<HMODULE> plugins;
u64 song_data_size = 1024 * 1024 * 64;
void *song_data;

std::string server       = "127.0.0.1";
std::string port         = "54430";
std::string chassisId    = "284111080000";
std::string shopId       = "TAIKO ARCADE LOADER";
std::string gameVerNum   = "00.00";
std::string countryCode  = "JPN";
char fullAddress[256]    = {'\0'};
char placeId[16]         = {'\0'};
char accessCode1[21]     = "00000000000000000001";
char accessCode2[21]     = "00000000000000000002";
char chipId1[33]         = "00000000000000000000000000000001";
char chipId2[33]         = "00000000000000000000000000000002";
bool windowed            = false;
bool autoIme             = false;
bool jpLayout            = false;
bool useLayeredFs        = false;
bool emulateUsio         = true;
bool emulateCardReader   = true;
bool emulateQr           = true;

HWND hGameWnd;
HOOK (i32, ShowMouse, PROC_ADDRESS ("user32.dll", "ShowCursor"), bool) { return originalShowMouse.call<i32> (true); }
HOOK (i32, ExitWindows, PROC_ADDRESS ("user32.dll", "ExitWindowsEx")) { ExitProcess (0); }
HOOK (HWND, CreateWindow, PROC_ADDRESS ("user32.dll", "CreateWindowExW"), DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
      i32 X, i32 Y, i32 nWidth, i32 nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    if (lpWindowName != NULL) {
        if (wcscmp (lpWindowName, L"Taiko") == 0) {
            if (windowed) dwStyle = WS_TILEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME;

            hGameWnd = originalCreateWindow.call<HWND> (dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu,
                                                        hInstance, lpParam);
            return hGameWnd;
        }
    }
    return originalCreateWindow.call<HWND> (dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance,
                                            lpParam);
}
HOOK (bool, SetWindowPosition, PROC_ADDRESS ("user32.dll", "SetWindowPos"), HWND hWnd, HWND hWndInsertAfter, i32 X, i32 Y, i32 cx, i32 cy,
      u32 uFlags) {
    if (hWnd == hGameWnd) {
        RECT rw, rc;
        GetWindowRect (hWnd, &rw);
        GetClientRect (hWnd, &rc);
        cx = (rw.right - rw.left) - (rc.right - rc.left) + cx;
        cy = (rw.bottom - rw.top) - (rc.bottom - rc.top) + cy;
    }
    return originalSetWindowPosition.call<bool> (hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

HOOK (void, ExitProcessHook, PROC_ADDRESS ("kernel32.dll", "ExitProcess"), u32 uExitCode) {
    bnusio::Close ();
    originalExitProcessHook.call<void> (uExitCode);
}

HOOK (i32, XinputGetState, PROC_ADDRESS ("xinput9_1_0.dll", "XInputGetState")) { return ERROR_DEVICE_NOT_CONNECTED; }
HOOK (i32, XinputSetState, PROC_ADDRESS ("xinput9_1_0.dll", "XInputSetState")) { return ERROR_DEVICE_NOT_CONNECTED; }
HOOK (i32, XinputGetCapabilites, PROC_ADDRESS ("xinput9_1_0.dll", "XInputGetCapabilities")) { return ERROR_DEVICE_NOT_CONNECTED; }

HOOK (i32, ssleay_Shutdown, PROC_ADDRESS ("ssleay32.dll", "SSL_shutdown")) { return 1; }

HOOK (i64, UsbFinderInitialize, PROC_ADDRESS ("nbamUsbFinder.dll", "nbamUsbFinderInitialize")) { return 0; }
HOOK (i64, UsbFinderRelease, PROC_ADDRESS ("nbamUsbFinder.dll", "nbamUsbFinderRelease")) { return 0; }
HOOK (i64, UsbFinderGetSerialNumber, PROC_ADDRESS ("nbamUsbFinder.dll", "nbamUsbFinderGetSerialNumber"), i32 a1, char *a2) {
    strcpy (a2, chassisId.c_str ());
    return 0;
}

HOOK (i32, ws2_getaddrinfo, PROC_ADDRESS ("ws2_32.dll", "getaddrinfo"), const char *node, char *service, void *hints, void *out) {
    return originalws2_getaddrinfo.call<i32> (server.c_str (), service, hints, out);
}

void
GetGameVersion () {
    wchar_t w_path[MAX_PATH];
    GetModuleFileNameW (nullptr, w_path, MAX_PATH);
    std::filesystem::path path (w_path);

    if (!std::filesystem::exists (path) || !path.has_filename ()) {
        MessageBoxA (nullptr, "Failed to find executable", nullptr, MB_OK);
        ExitProcess (0);
    }

    std::ifstream stream (path, std::ios::binary);
    if (!stream.is_open ()) {
        MessageBoxA (nullptr, "Failed to read executable", nullptr, MB_OK);
        ExitProcess (0);
    }

    stream.seekg (0, std::ifstream::end);
    size_t length = stream.tellg ();
    stream.seekg (0, std::ifstream::beg);

    char *buf = (char *)calloc (length + 1, sizeof (char));
    stream.read (buf, length);

    gameVersion = (GameVersion)XXH64 (buf, length, 0);

    stream.close ();
    free (buf);

    switch (gameVersion) {
    case GameVersion::JPN00:
    case GameVersion::JPN08:
    case GameVersion::JPN39:
    case GameVersion::CHN00: break;
    default: MessageBoxA (nullptr, "Unknown game version", nullptr, MB_OK); ExitProcess (0);
    }
}

void
CreateCard () {
    const char hexCharacterTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char buf[64]                   = {0};
    srand (time (nullptr));

    std::generate (buf, buf + 20, [&] () { return hexCharacterTable[rand () % 10]; });
    WritePrivateProfileStringA ("card", "accessCode1", buf, ".\\card.ini");
    std::generate (buf, buf + 32, [&] () { return hexCharacterTable[rand () % 16]; });
    WritePrivateProfileStringA ("card", "chipId1", buf, ".\\card.ini");
    std::generate (buf, buf + 20, [&] () { return hexCharacterTable[rand () % 10]; });
    WritePrivateProfileStringA ("card", "accessCode2", buf, ".\\card.ini");
    std::generate (buf, buf + 32, [&] () { return hexCharacterTable[rand () % 16]; });
    WritePrivateProfileStringA ("card", "chipId2", buf, ".\\card.ini");
}

BOOL
DllMain (HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        // This is bad, dont do this
        // I/O in DllMain can easily cause a deadlock

        std::string version = "auto";
        auto configPath     = std::filesystem::current_path () / "config.toml";
        std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
        if (config_ptr) {
            toml_table_t *config = config_ptr.get ();
            auto amauth          = openConfigSection (config, "amauth");
            if (amauth) {
                server      = readConfigString (amauth, "server", server);
                port        = readConfigString (amauth, "port", port);
                chassisId   = readConfigString (amauth, "chassis_id", chassisId);
                shopId      = readConfigString (amauth, "shop_id", shopId);
                gameVerNum  = readConfigString (amauth, "game_ver", gameVerNum);
                countryCode = readConfigString (amauth, "country_code", countryCode);

                std::strcat (fullAddress, server.c_str ());
                if (port != "") {
                    std::strcat (fullAddress, ":");
                    std::strcat (fullAddress, port.c_str ());
                }

                std::strcat (placeId, countryCode.c_str ());
                std::strcat (placeId, "0FF0");
            }
            auto patches = openConfigSection (config, "patches");
            if (patches) version = readConfigString (patches, "version", version);
            auto emulation = openConfigSection (config, "emulation");
            if (emulation) {
                emulateUsio       = readConfigBool (emulation, "usio", emulateUsio);
                emulateCardReader = readConfigBool (emulation, "card_reader", emulateCardReader);
                emulateQr         = readConfigBool (emulation, "qr", emulateQr);
            }
            auto graphics = openConfigSection (config, "graphics");
            if (graphics) windowed = readConfigBool (graphics, "windowed", windowed);
            auto keyboard = openConfigSection (config, "keyboard");
            if (keyboard) {
                autoIme  = readConfigBool (keyboard, "auto_ime", autoIme);
                jpLayout = readConfigBool (keyboard, "jp_layout", jpLayout);
            }
        }

        if (version == "auto") {
            GetGameVersion ();
        } else if (version == "JPN00") {
            gameVersion = GameVersion::JPN00;
        } else if (version == "JPN08") {
            gameVersion = GameVersion::JPN08;
        } else if (version == "JPN39") {
            gameVersion = GameVersion::JPN39;
        } else if (version == "CHN00") {
            gameVersion = GameVersion::CHN00;
        } else {
            MessageBoxA (nullptr, "Unknown patch version", nullptr, MB_OK);
            ExitProcess (0);
        }

        auto pluginPath = std::filesystem::current_path () / "plugins";

        if (std::filesystem::exists (pluginPath)) {
            for (const auto &entry : std::filesystem::directory_iterator (pluginPath)) {
                if (entry.path ().extension () == ".dll") {
                    auto name       = entry.path ().wstring ();
                    HMODULE hModule = LoadLibraryW (name.c_str ());
                    if (!hModule) {
                        wchar_t buf[128];
                        wsprintfW (buf, L"Failed to load plugin %ls", name.c_str ());
                        MessageBoxW (0, buf, name.c_str (), MB_ICONERROR);
                    } else {
                        plugins.push_back (hModule);
                    }
                }
            }
        }

        if (!std::filesystem::exists (".\\card.ini")) CreateCard ();
        GetPrivateProfileStringA ("card", "accessCode1", accessCode1, accessCode1, 21, ".\\card.ini");
        GetPrivateProfileStringA ("card", "chipId1", chipId1, chipId1, 33, ".\\card.ini");
        GetPrivateProfileStringA ("card", "accessCode2", accessCode2, accessCode2, 21, ".\\card.ini");
        GetPrivateProfileStringA ("card", "chipId2", chipId2, chipId2, 33, ".\\card.ini");

        INSTALL_HOOK (ShowMouse);
        INSTALL_HOOK (ExitWindows);
        INSTALL_HOOK (CreateWindow);
        INSTALL_HOOK (SetWindowPosition);

        INSTALL_HOOK (ExitProcessHook);

        INSTALL_HOOK (XinputGetState);
        INSTALL_HOOK (XinputSetState);
        INSTALL_HOOK (XinputGetCapabilites);

        INSTALL_HOOK (ssleay_Shutdown);

        INSTALL_HOOK (UsbFinderInitialize);
        INSTALL_HOOK (UsbFinderRelease);
        INSTALL_HOOK (UsbFinderGetSerialNumber);

        INSTALL_HOOK (ws2_getaddrinfo);

        bnusio::Init ();

        switch (gameVersion) {
        case GameVersion::UNKNOWN: break;
        case GameVersion::JPN00: patches::JPN00::Init (); break;
        case GameVersion::JPN08: patches::JPN08::Init (); break;
        case GameVersion::JPN39: patches::JPN39::Init (); break;
        case GameVersion::CHN00: patches::CHN00::Init (); break;
        }

        patches::Qr::Init ();
        patches::Audio::Init ();
        patches::Dxgi::Init ();
        patches::AmAuth::Init ();
        patches::LayeredFs::Init ();
        patches::TestMode::Init ();
    }
    return true;
}
