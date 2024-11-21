#include "constants.h"
#include "helpers.h"
#include "patches.h"

extern GameVersion gameVersion;

namespace patches::Audio {

typedef struct nusc_init_config {
    uint32_t sample_rate;
    uint32_t buffer_size;
    uint32_t device_mode;
    uint32_t channel_count;
    const char *asio_driver_name;
    bool wasapi_disable_com;
    bool wasapi_exclusive;
    uint32_t wasapi_exclusive_buffer_size;
    void *wasapi_audioses;
} nusc_init_config_t;

bool wasapiShared      = true;
bool asio              = false;
std::string asioDriver = "";

HOOK_DYNAMIC (i64, NUSCDeviceInit, void *a1, nusc_init_config_t *a2, nusc_init_config_t *a3, void *a4) {
    LogMessage (LOG_LEVEL_INFO, (std::string ("Device mode is ") + (asio ? "ASIO" : (wasapiShared ? "wasapi shared" : "wasapi exclusive"))).c_str ());
    if (asio) LogMessage (LOG_LEVEL_INFO, (std::string ("ASIO driver is ") + asioDriver).c_str ());
    a2->device_mode      = asio;
    a2->asio_driver_name = asio ? asioDriver.c_str () : "";
    a2->wasapi_exclusive = asio ? 1 : wasapiShared ? 0 : 1;
    return originalNUSCDeviceInit.call<i64> (a1, a2, a3, a4);
}
HOOK_DYNAMIC (bool, LoadASIODriver, void *a1, const char *a2) {
    auto result = originalLoadASIODriver.call<bool> (a1, a2);
    if (!result) {
        LogMessage (LOG_LEVEL_ERROR, (std::string ("Failed to load ASIO driver ") + asioDriver).c_str ());
        MessageBoxA (nullptr, "Failed to load ASIO driver", nullptr, MB_OK);
        ExitProcess (0);
    }
    return result;
}

void
Init () {
    LogMessage (LOG_LEVEL_DEBUG, "Init Audio patches");

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        auto audio = openConfigSection (config_ptr.get (), "audio");
        if (audio) {
            wasapiShared = readConfigBool (audio, "wasapi_shared", wasapiShared);
            asio         = readConfigBool (audio, "asio", asio);
            asioDriver   = readConfigString (audio, "asio_driver", asioDriver);
        }
    }

    switch (gameVersion) {
    case GameVersion::JPN00: {
        INSTALL_HOOK_DYNAMIC (NUSCDeviceInit, ASLR (0x140552160));
        INSTALL_HOOK_DYNAMIC (LoadASIODriver, ASLR (0x14055A950));
        break;
    }
    case GameVersion::JPN08: {
        INSTALL_HOOK_DYNAMIC (NUSCDeviceInit, ASLR (0x140692E00));
        INSTALL_HOOK_DYNAMIC (LoadASIODriver, ASLR (0x14069B750));
        break;
    }
    case GameVersion::JPN39: {
        INSTALL_HOOK_DYNAMIC (NUSCDeviceInit, ASLR (0x1407C8620));
        INSTALL_HOOK_DYNAMIC (LoadASIODriver, ASLR (0x1407D0F70));
        break;
    }
    case GameVersion::CHN00: {
        INSTALL_HOOK_DYNAMIC (NUSCDeviceInit, ASLR (0x140777F70));
        INSTALL_HOOK_DYNAMIC (LoadASIODriver, ASLR (0x1407808C0));
        break;
    }
    default: {
        break;
    }
    }
}
} // namespace patches::Audio
