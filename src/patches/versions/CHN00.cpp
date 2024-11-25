#include "helpers.h"
#include "../patches.h"
#include <safetyhook.hpp>

extern std::string chassisId;

namespace patches::CHN00 {
int language = 0;
u8 *haspBuffer;
HOOK (i32, HaspDecrypt, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_decrypt")) { return 0; }
HOOK (i32, HaspEncrypt, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_encrypt")) { return 0; }
HOOK (i32, HaspLogout, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_logout")) { return 0; }
HOOK (i32, HaspWrite, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_write")) { return 0; }
HOOK (i32, HaspLogin, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_login"), i32, char *, i32 *id) {
    *id = 1;
    return 0;
}
HOOK (i32, HaspGetInfo, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_get_info"), const char *, const char *, void *, const char **a4) {
    *a4 = "type=\"HASP-HL\"";
    return 0;
}
HOOK (i32, HaspRead, PROC_ADDRESS ("hasp_windows_x64.dll", "hasp_read"), i32, i32, i32 offset, i32 length, void *buffer) {
    memcpy (buffer, haspBuffer + offset, length);
    return 0;
}

FUNCTION_PTR (i64, lua_settop, PROC_ADDRESS ("lua51.dll", "lua_settop"), u64, u64);
FUNCTION_PTR (i64, lua_pushboolean, PROC_ADDRESS ("lua51.dll", "lua_pushboolean"), u64, u64);
FUNCTION_PTR (i64, lua_pushstring, PROC_ADDRESS ("lua51.dll", "lua_pushstring"), u64, u64);

i64
lua_pushtrue (i64 a1) {
    lua_settop (a1, 0);
    lua_pushboolean (a1, 1);
    return 1;
}

HOOK (i64, AvailableMode_Dani_AI, ASLR (0x1401AC550), i64 a1) {
    LogMessage (LOG_LEVEL_HOOKS, "AvailableMode_Dani_AI was called");
    return lua_pushtrue (a1);
}
HOOK (i64, AvailableMode_Collabo025, ASLR (0x1402BFF70), i64 *, i64 a2) {
    LogMessage (LOG_LEVEL_HOOKS, "AvailableMode_Collabo025 was called");
    return lua_pushtrue (a2);
}
HOOK (i64, AvailableMode_Collabo026, ASLR (0x1402BC9B0), i64 a1) {
    LogMessage (LOG_LEVEL_HOOKS, "AvailableMode_Collabo026 was called");
    return lua_pushtrue (a1);
}

HOOK (i64, GetLanguage, ASLR (0x140023720), i64 a1) {
    LogMessage (LOG_LEVEL_HOOKS, "GetLanguage was called");
    auto result = originalGetLanguage (a1);
    language    = *((u32 *)result);
    return result;
}
HOOK (i64, GetRegionLanguage, ASLR (0x1401AC300), i64 a1) {
    LogMessage (LOG_LEVEL_HOOKS, "GetRegionLanguage was called");
    lua_settop (a1, 0);
    lua_pushstring (a1, (u64)languageStr (language));
    return 1;
}
HOOK (i64, GetCabinetLanguage, ASLR (0x1401AF270), i64, i64 a2) {
    LogMessage (LOG_LEVEL_HOOKS, "GetCabinetLanguage was called");
    lua_settop (a2, 0);
    lua_pushstring (a2, (u64)languageStr (language));
    return 1;
}

HOOK_DYNAMIC (char, AMFWTerminate, i64) {
    LogMessage (LOG_LEVEL_HOOKS, "AMFWTerminate was called");
    return 0;
}

const i32 datatableBufferSize = 1024 * 1024 * 12;
safetyhook::Allocation datatableBuffer1;
safetyhook::Allocation datatableBuffer2;
safetyhook::Allocation datatableBuffer3;
const std::vector<uintptr_t> datatableBuffer1Addresses = {0x140093430, 0x1400934A1, 0x1400934CB, 0x14009353C};
const std::vector<uintptr_t> datatableBuffer2Addresses = {0x14009341C, 0x14009354B, 0x14009357E};
const std::vector<uintptr_t> datatableBuffer3Addresses = {0x14009356F, 0x140093585, 0x1400935AF};
const std::vector<uintptr_t> memsetSizeAddresses       = {0x140093416, 0x14009342A, 0x140093569};

void
AllocateStaticBufferNear (void *target_address, size_t size, safetyhook::Allocation *newBuffer) {
    auto allocator                = safetyhook::Allocator::global ();
    std::vector desired_addresses = {(uint8_t *)target_address};
    auto allocation_result        = allocator->allocate_near (desired_addresses, size);
    if (allocation_result.has_value ()) *newBuffer = std::move (*allocation_result);
}

void
ReplaceLeaBufferAddress (const std::vector<uintptr_t> &bufferAddresses, void *newBufferAddress) {
    for (auto bufferAddress : bufferAddresses) {
        uintptr_t lea_instruction_dst = ASLR (bufferAddress) + 3;
        uintptr_t lea_instruction_end = ASLR (bufferAddress) + 7;
        intptr_t offset               = (intptr_t)newBufferAddress - lea_instruction_end;
        WRITE_MEMORY (lea_instruction_dst, i32, (i32)offset);
    }
}

void
Init () {
    i32 xRes            = 1920;
    i32 yRes            = 1080;
    bool vsync          = false;
    bool unlockSongs    = true;
    bool fixLanguage    = false;
    bool demoMovie      = true;
    bool modeCollabo025 = false;
    bool modeCollabo026 = false;

    haspBuffer = (u8 *)malloc (0xD40);
    memset (haspBuffer, 0, 0xD40);
    strcpy ((char *)(haspBuffer + 0xD00), chassisId.c_str ());
    u8 crc = 0;
    for (int i = 0; i < 62; i++)
        crc += haspBuffer[0xD00 + i];
    haspBuffer[0xD3E] = crc;
    haspBuffer[0xD3F] = haspBuffer[0xD3E] ^ 0xFF;

    INSTALL_HOOK (HaspDecrypt);
    INSTALL_HOOK (HaspEncrypt);
    INSTALL_HOOK (HaspLogout);
    INSTALL_HOOK (HaspWrite);
    INSTALL_HOOK (HaspLogin);
    INSTALL_HOOK (HaspGetInfo);
    INSTALL_HOOK (HaspRead);

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        auto patches = openConfigSection (config_ptr.get (), "patches");
        if (patches) {
            unlockSongs = readConfigBool (patches, "unlock_songs", unlockSongs);
            auto chn00  = openConfigSection (patches, "chn00");
            if (chn00) {
                fixLanguage    = readConfigBool (chn00, "fix_language", fixLanguage);
                demoMovie      = readConfigBool (chn00, "demo_movie", demoMovie);
                modeCollabo025 = readConfigBool (chn00, "mode_collabo025", modeCollabo025);
                modeCollabo026 = readConfigBool (chn00, "mode_collabo026", modeCollabo026);
            }
        }

        auto graphics = openConfigSection (config_ptr.get (), "graphics");
        if (graphics) {
            auto res = openConfigSection (graphics, "res");
            if (res) {
                xRes = readConfigInt (res, "x", xRes);
                yRes = readConfigInt (res, "y", yRes);
            }
            vsync = readConfigBool (graphics, "vsync", vsync);
        }
    }

    // Apply common config patch
    WRITE_MEMORY (ASLR (0x1404A4ED3), i32, xRes);
    WRITE_MEMORY (ASLR (0x1404A4EDA), i32, yRes);
    if (!vsync) WRITE_MEMORY (ASLR (0x1405FC5B9), u8, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90);
    if (unlockSongs) WRITE_MEMORY (ASLR (0x140425BCD), u8, 0xB0, 0x01);

    // Bypass errors
    WRITE_MEMORY (ASLR (0x14003F690), u8, 0xC3);

    // Use TLS v1.2
    WRITE_MEMORY (ASLR (0x140369662), u8, 0x10);

    // Disable SSLVerify
    WRITE_MEMORY (ASLR (0x14034C182), u8, 0x00);

    // Move various files to current directory
    WRITE_MEMORY (ASLR (0x140C33C40), char, "./"); // F:/
    WRITE_MEMORY (ASLR (0x140C33C44), char, "./"); // G:/
    WRITE_MEMORY (ASLR (0x140C7B158), char, ".\\SettingChina1.bin");
    WRITE_MEMORY (ASLR (0x140C7B2B8), char, ".\\SettingChina1.bin");
    WRITE_MEMORY (ASLR (0x140C7B2A0), char, ".\\SettingChina2.bin");
    WRITE_MEMORY (ASLR (0x140C3CF58), char, ".\\TournamentData\\PlayData\\TournamentPlayData.dat");
    WRITE_MEMORY (ASLR (0x140C3CFC8), char, ".\\TournamentData\\PlayData\\TournamentPlayData.dat");
    WRITE_MEMORY (ASLR (0x140C3CF90), char, ".\\TournamentData\\InfoData\\TournamentInfoData.dat");
    WRITE_MEMORY (ASLR (0x140C3D000), char, ".\\TournamentData\\InfoData\\TournamentInfoData.dat");

    // Remove datatable size limit
    {
        for (auto address : memsetSizeAddresses)
            WRITE_MEMORY (ASLR (address) + 2, i32, datatableBufferSize);

        auto bufferBase = MODULE_HANDLE - 0x03000000;
        AllocateStaticBufferNear ((void *)bufferBase, datatableBufferSize, &datatableBuffer1);
        bufferBase += datatableBufferSize;
        AllocateStaticBufferNear ((void *)bufferBase, datatableBufferSize, &datatableBuffer2);
        bufferBase += datatableBufferSize;
        AllocateStaticBufferNear ((void *)bufferBase, datatableBufferSize, &datatableBuffer3);

        ReplaceLeaBufferAddress (datatableBuffer1Addresses, datatableBuffer1.data ());
        ReplaceLeaBufferAddress (datatableBuffer2Addresses, datatableBuffer2.data ());
        ReplaceLeaBufferAddress (datatableBuffer3Addresses, datatableBuffer3.data ());
    }

    // Fix language
    if (fixLanguage) {
        INSTALL_HOOK (GetLanguage);
        INSTALL_HOOK (GetRegionLanguage);
        INSTALL_HOOK (GetCabinetLanguage);
    }

    // Disable demo movie
    if (!demoMovie) WRITE_MEMORY (ASLR (0x14047D387), u8, 0x00);

    // Enable mode
    INSTALL_HOOK (AvailableMode_Dani_AI);
    if (modeCollabo025) INSTALL_HOOK (AvailableMode_Collabo025);
    if (modeCollabo026) INSTALL_HOOK (AvailableMode_Collabo026);

    // Disable live check
    auto amHandle = (u64)GetModuleHandle ("AMFrameWork.dll");
    INSTALL_HOOK_DYNAMIC (AMFWTerminate, (void *)(amHandle + 0x25A00));

    // Move various files to current directory
    WRITE_MEMORY (amHandle + 0xC652, u8, 0xEB);  // CreditLogPathA
    WRITE_MEMORY (amHandle + 0xC819, u8, 0xEB);  // CreditLogPathB
    WRITE_MEMORY (amHandle + 0x243BA, u8, 0xEB); // ErrorLogPathA
    WRITE_MEMORY (amHandle + 0x24539, u8, 0xEB); // ErrorLogPathB
    WRITE_MEMORY (amHandle + 0x24901, u8, 0xEB); // CommonLogPathA
    WRITE_MEMORY (amHandle + 0x24A85, u8, 0xEB); // CommonLogPathB
    WRITE_NOP (amHandle + 0x24DD1, 0x05);        // BackupDataPathA
    WRITE_NOP (amHandle + 0x24E47, 0x05);        // BackupDataPathB
}
} // namespace patches::CHN00
