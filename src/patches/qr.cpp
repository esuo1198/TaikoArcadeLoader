#include "constants.h"
#include "helpers.h"
#include "poll.h"
#include <ReadBarcode.h>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

extern GameVersion gameVersion;
extern Keybindings CARD_INSERT_1;
extern Keybindings CARD_INSERT_2;
extern Keybindings QR_DATA_READ;
extern Keybindings QR_IMAGE_READ;
extern char accessCode1[21];
extern char accessCode2[21];
extern std::vector<HMODULE> plugins;
extern bool emulateQr;

typedef void event ();
typedef void initQrEvent (GameVersion gameVersion);
typedef bool checkQrEvent ();
typedef int getQrEvent (int, unsigned char *);

namespace patches::Qr {

enum class State { Ready, CopyWait };
enum class Mode { Card, Data, Image, Plugin };
State gState = State::Ready;
Mode gMode   = Mode::Card;
HMODULE gPlugin;
std::string accessCode;

std::vector<HMODULE> qrPlugins;
bool qrPluginRegistered = false;

HOOK_DYNAMIC (char, QrInit, i64) { return 1; }
HOOK_DYNAMIC (char, QrClose, i64) { return 1; }
HOOK_DYNAMIC (char, QrRead, i64 a1) {
    *reinterpret_cast<DWORD *> (a1 + 40) = 1;
    *reinterpret_cast<DWORD *> (a1 + 16) = 1;
    *reinterpret_cast<BYTE *> (a1 + 112) = 0;
    return 1;
}
HOOK_DYNAMIC (i64, CallQrUnknown, i64) { return 1; }
HOOK_DYNAMIC (bool, Send1, i64 a1) {
    *reinterpret_cast<BYTE *> (a1 + 88) = 1;
    *reinterpret_cast<i64 *> (a1 + 32)  = *reinterpret_cast<i64 *> (a1 + 24);
    *reinterpret_cast<WORD *> (a1 + 89) = 0;
    return true;
}
HOOK_DYNAMIC (bool, Send2, i64 a1) {
    *reinterpret_cast<WORD *> (a1 + 88) = 0;
    *reinterpret_cast<BYTE *> (a1 + 90) = 0;
    return true;
}
HOOK_DYNAMIC (bool, Send3, i64, char) { return true; }
HOOK_DYNAMIC (bool, Send4, i64, const void *, i64) { return true; }
HOOK_DYNAMIC (i64, CopyData, i64, void *dest, int length) {
    if (gState == State::CopyWait) {
        // std::cout << "Copy data, length: " << length << std::endl;
        auto configPath = std::filesystem::current_path () / "config.toml";
        std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);

        if (gMode == Mode::Card) {
            memcpy (dest, accessCode.c_str (), accessCode.size () + 1);
            gState = State::Ready;
            return accessCode.size () + 1;
        } else if (gMode == Mode::Data) {
            std::string serial = "";
            u16 type           = 0;
            std::vector<i64> songNoes;

            if (config_ptr) {
                if (auto qr = openConfigSection (config_ptr.get (), "qr")) {
                    if (auto data = openConfigSection (qr, "data")) {
                        serial   = readConfigString (data, "serial", "");
                        type     = (u16)readConfigInt (data, "type", 0);
                        songNoes = readConfigIntArray (data, "song_no", songNoes);
                    }
                }
            }

            BYTE serial_length           = static_cast<BYTE> (serial.size ());
            std::vector<BYTE> byteBuffer = {0x53, 0x31, 0x32, 0x00, 0x00, 0xFF, 0xFF, serial_length, 0x01, 0x00};

            for (char c : serial)
                byteBuffer.push_back (static_cast<BYTE> (c));

            if (type == 5) {
                std::vector<BYTE> folderData = {0xFF, 0xFF};

                folderData.push_back (static_cast<u8> (songNoes.size ()) * 2);

                folderData.push_back (static_cast<u8> (type & 0xFF));
                folderData.push_back (static_cast<u8> ((type >> 8) & 0xFF));

                for (i64 songNo : songNoes) {
                    folderData.push_back (static_cast<u8> (songNo & 0xFF));
                    folderData.push_back (static_cast<u8> ((songNo >> 8) & 0xFF));
                }

                for (auto c : folderData)
                    byteBuffer.push_back (c);
            }

            byteBuffer.push_back (0xEE);
            byteBuffer.push_back (0xFF);

            std::stringstream hexStream;
            for (auto byteData : byteBuffer)
                hexStream << std::hex << std::uppercase << std::setfill ('0') << std::setw (2) << static_cast<int> (byteData) << " ";
            LogMessage (LogLevel::INFO, ("Data dump: " + hexStream.str ()).c_str ());

            memcpy (dest, byteBuffer.data (), byteBuffer.size ());
            gState = State::Ready;
            return byteBuffer.size ();
        } else if (gMode == Mode::Image) {
            std::string imagePath = "";

            if (config_ptr) {
                if (auto qr = openConfigSection (config_ptr.get (), "qr")) imagePath = readConfigString (qr, "image_path", "");
            }

            std::u8string u8PathStr (imagePath.begin (), imagePath.end ());
            std::filesystem::path u8Path (u8PathStr);
            if (!std::filesystem::is_regular_file (u8Path)) {
                LogMessage (LogLevel::ERROR, ("Failed to open image: " + u8Path.string () + " (file not found)").c_str ());
                gState = State::Ready;
                return 0;
            }

            int width, height, channels;
            std::unique_ptr<stbi_uc, void (*) (void *)> buffer (stbi_load (u8Path.string ().c_str (), &width, &height, &channels, 3),
                                                                stbi_image_free);
            if (!buffer) {
                LogMessage (LogLevel::ERROR, ("Failed to read image: " + u8Path.string () + " (" + stbi_failure_reason () + ")").c_str ());
                gState = State::Ready;
                return 0;
            }

            ZXing::ImageView image{buffer.get (), width, height, ZXing::ImageFormat::RGB};
            auto result = ReadBarcode (image);
            if (!result.isValid ()) {
                LogMessage (LogLevel::ERROR, ("Failed to read QR: " + imagePath + " (" + ToString (result.error ()) + ")").c_str ());
                gState = State::Ready;
                return 0;
            }

            // std::cout << "Valid" << std::endl;
            auto byteData = result.bytes ();
            // std::cout << ZXing::ToHex (byteData) << std::endl;
            auto dataSize = byteData.size ();

            memcpy (dest, byteData.data (), dataSize);
            gState = State::Ready;
            return dataSize;
        } else if (gMode == Mode::Plugin) {
            if (FARPROC getEvent = GetProcAddress (gPlugin, "GetQr")) {
                std::vector<unsigned char> plugin_data (length);
                int buf_len = reinterpret_cast<getQrEvent *> (getEvent) (length, plugin_data.data ());
                if (0 < buf_len && buf_len <= length) {
                    std::stringstream hexStream;
                    for (int i = 0; i < buf_len; i++)
                        hexStream << std::hex << std::uppercase << std::setfill ('0') << std::setw (2) << static_cast<int> (plugin_data[i]) << " ";
                    LogMessage (LogLevel::INFO, ("QR dump: " + hexStream.str ()).c_str ());
                    memcpy (dest, plugin_data.data (), buf_len);
                } else {
                    LogMessage (LogLevel::ERROR, ("QR discard! Length invalid: " + std::to_string (buf_len) + ", valid range: 0~").c_str ());
                }
                gState = State::Ready;
                return buf_len;
            } else {
                gState = State::Ready;
                return 0;
            }
        }
    } else if (qrPluginRegistered) {
        for (auto plugin : qrPlugins)
            if (FARPROC usingQrEvent = GetProcAddress (plugin, "UsingQr")) ((event *)usingQrEvent) ();
    }
    return 0;
}

void
Update () {
    if (!emulateQr) return;
    if (gState == State::Ready) {
        // std::cout << "Insert" << std::endl;
        if (IsButtonTapped (CARD_INSERT_1)) {
            if (gameVersion != GameVersion::CHN00) return;
            accessCode = "BNTTCNID";
            accessCode += accessCode1;
            gState = State::CopyWait;
            gMode  = Mode::Card;
        } else if (IsButtonTapped (CARD_INSERT_2)) {
            if (gameVersion != GameVersion::CHN00) return;
            accessCode = "BNTTCNID";
            accessCode += accessCode2;
            gState = State::CopyWait;
            gMode  = Mode::Card;
        } else if (IsButtonTapped (QR_DATA_READ)) {
            gState = State::CopyWait;
            gMode  = Mode::Data;
        } else if (IsButtonTapped (QR_IMAGE_READ)) {
            gState = State::CopyWait;
            gMode  = Mode::Image;
        } else if (qrPluginRegistered) {
            for (const auto plugin : qrPlugins) {
                const FARPROC checkEvent = GetProcAddress (plugin, "CheckQr");
                if (checkEvent && ((checkQrEvent *)checkEvent) ()) {
                    gState  = State::CopyWait;
                    gMode   = Mode::Plugin;
                    gPlugin = plugin;
                    break;
                }
            }
        }
    }
}

void
Init () {
    LogMessage (LogLevel::INFO, "Init Qr patches");

    if (!emulateQr) {
        LogMessage (LogLevel::WARN, "QR emulation disabled");
        return;
    }

    for (auto plugin : plugins) {
        const FARPROC initEvent = GetProcAddress (plugin, "InitQr");
        if (initEvent) ((initQrEvent *)initEvent) (gameVersion);

        const FARPROC usingQrEvent = GetProcAddress (plugin, "UsingQr");
        if (usingQrEvent) qrPlugins.push_back (plugin);
    }
    if (qrPlugins.size () > 0) {

        LogMessage (LogLevel::INFO, "QR plugin found!");
        qrPluginRegistered = true;
    }

    SetConsoleOutputCP (CP_UTF8);
    const auto amHandle = (u64)GetModuleHandle ("AMFrameWork.dll");
    switch (gameVersion) {
    case GameVersion::JPN00: {
        INSTALL_HOOK_DYNAMIC (QrInit, (LPVOID)(amHandle + 0x1B3E0));
        INSTALL_HOOK_DYNAMIC (QrClose, (LPVOID)(amHandle + 0x1B5B0));
        INSTALL_HOOK_DYNAMIC (QrRead, (LPVOID)(amHandle + 0x1B600));
        INSTALL_HOOK_DYNAMIC (CallQrUnknown, (LPVOID)(amHandle + 0xFD40));
        INSTALL_HOOK_DYNAMIC (Send1, (LPVOID)(amHandle + 0x1BBB0));
        INSTALL_HOOK_DYNAMIC (Send2, (LPVOID)(amHandle + 0x1BBF0));
        INSTALL_HOOK_DYNAMIC (Send3, (LPVOID)(amHandle + 0x1BC60));
        // JPN00 has no Send4
        INSTALL_HOOK_DYNAMIC (CopyData, (LPVOID)(amHandle + 0x1BC30));
        break;
    }
    case GameVersion::JPN08: {
        INSTALL_HOOK_DYNAMIC (QrInit, reinterpret_cast<LPVOID> (amHandle + 0x1BA00));
        INSTALL_HOOK_DYNAMIC (QrClose, reinterpret_cast<LPVOID> (amHandle + 0x1BBD0));
        INSTALL_HOOK_DYNAMIC (QrRead, (LPVOID)(amHandle + 0x1BC20));
        INSTALL_HOOK_DYNAMIC (CallQrUnknown, (LPVOID)(amHandle + 0xFD40));
        INSTALL_HOOK_DYNAMIC (Send1, (LPVOID)(amHandle + 0x1C220));
        INSTALL_HOOK_DYNAMIC (Send2, (LPVOID)(amHandle + 0x1C260));
        INSTALL_HOOK_DYNAMIC (Send3, (LPVOID)(amHandle + 0x1C2D0));
        // JPN08 has no Send4
        INSTALL_HOOK_DYNAMIC (CopyData, (LPVOID)(amHandle + 0x1C2A0));
        break;
    }
    case GameVersion::JPN39: {
        INSTALL_HOOK_DYNAMIC (QrInit, (LPVOID)(amHandle + 0x1EDC0));
        INSTALL_HOOK_DYNAMIC (QrClose, (LPVOID)(amHandle + 0x1EF60));
        INSTALL_HOOK_DYNAMIC (QrRead, (LPVOID)(amHandle + 0x1EFB0));
        INSTALL_HOOK_DYNAMIC (CallQrUnknown, (LPVOID)(amHandle + 0x11A70));
        INSTALL_HOOK_DYNAMIC (Send1, (LPVOID)(amHandle + 0x1F5B0));
        INSTALL_HOOK_DYNAMIC (Send2, (LPVOID)(amHandle + 0x1F5F0));
        INSTALL_HOOK_DYNAMIC (Send3, (LPVOID)(amHandle + 0x1F660));
        INSTALL_HOOK_DYNAMIC (Send4, (LPVOID)(amHandle + 0x1F690));
        INSTALL_HOOK_DYNAMIC (CopyData, (LPVOID)(amHandle + 0x1F630));
        break;
    }
    case GameVersion::CHN00: {
        INSTALL_HOOK_DYNAMIC (QrInit, (LPVOID)(amHandle + 0x161B0));
        INSTALL_HOOK_DYNAMIC (QrClose, (LPVOID)(amHandle + 0x16350));
        INSTALL_HOOK_DYNAMIC (QrRead, (LPVOID)(amHandle + 0x163A0));
        INSTALL_HOOK_DYNAMIC (CallQrUnknown, (LPVOID)(amHandle + 0x8F60));
        INSTALL_HOOK_DYNAMIC (Send1, (LPVOID)(amHandle + 0x16940));
        INSTALL_HOOK_DYNAMIC (Send2, (LPVOID)(amHandle + 0x16990));
        INSTALL_HOOK_DYNAMIC (Send3, (LPVOID)(amHandle + 0x16A00));
        INSTALL_HOOK_DYNAMIC (Send4, (LPVOID)(amHandle + 0x16A30));
        INSTALL_HOOK_DYNAMIC (CopyData, (LPVOID)(amHandle + 0x169D0));
        break;
    }
    default: {
        break;
    }
    }
}
} // namespace patches::Qr
