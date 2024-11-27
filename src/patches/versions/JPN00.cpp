#include "helpers.h"
#include "../patches.h"

namespace patches::JPN00 {

HOOK_DYNAMIC (char, AMFWTerminate, i64) { return 0; }

constexpr i32 datatableBufferSize = 1024 * 1024 * 12;
safetyhook::Allocation datatableBuffer;
const std::vector<uintptr_t> datatableBufferAddresses
    = {0x14005A418, 0x14005A445, 0x14005A778, 0x14005A7A5, 0x14005AD58, 0x14005AD85, 0x14005B1F2, 0x14005B221, 0x14005B438, 0x14005B465,
       0x14005B7D8, 0x14005B805, 0x14005BA59, 0x14005BA88, 0x14005BD78, 0x14005BDA7, 0x14005C1B3, 0x14005C1E2, 0x14005CC52, 0x14005CC81,
       0x14005D348, 0x14005D375, 0x14005D668, 0x14005D695, 0x14005D9D2, 0x14005DA04, 0x14005EA04, 0x14005EA33, 0x14005EC18, 0x14005EC45,
       0x14005EEE4, 0x14005EF10, 0x14005F158, 0x14005F185, 0x14005F47C, 0x14005F4A9, 0x140279F8D};
const std::vector<uintptr_t> memsetSizeAddresses
    = {0x14005A412, 0x14005A772, 0x14005AD52, 0x14005B1EC, 0x14005B432, 0x14005B7D2, 0x14005BA53, 0x14005BD72, 0x14005C1AD,
       0x14005CC4C, 0x14005D342, 0x14005D662, 0x14005D9CC, 0x14005E9FE, 0x14005EC12, 0x14005EEDE, 0x14005F152, 0x14005F476};

void
AllocateStaticBufferNear (void *target_address, const size_t size, safetyhook::Allocation *newBuffer) {
    const auto allocator                = safetyhook::Allocator::global ();
    const std::vector desired_addresses = {static_cast<u8 *> (target_address)};
    if (auto allocation_result = allocator->allocate_near (desired_addresses, size); allocation_result.has_value ())
        *newBuffer = std::move (*allocation_result);
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
    LogMessage (LogLevel::INFO, "Init JNP00 patches");
    i32 xRes         = 1920;
    i32 yRes         = 1080;
    bool vsync       = false;
    bool unlockSongs = true;

    const auto configPath = std::filesystem::current_path () / "config.toml";
    const std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        if (const auto patches = openConfigSection (config_ptr.get (), "patches"))
            unlockSongs = readConfigBool (patches, "unlock_songs", unlockSongs);

        if (const auto graphics = openConfigSection (config_ptr.get (), "graphics")) {
            if (const auto res = openConfigSection (graphics, "res")) {
                xRes = (i32)readConfigInt (res, "x", xRes);
                yRes = (i32)readConfigInt (res, "y", yRes);
            }
            vsync = readConfigBool (graphics, "vsync", vsync);
        }
    }

    // Apply common config patch
    WRITE_MEMORY (ASLR (0x140224B2B), i32, xRes);
    WRITE_MEMORY (ASLR (0x140224B32), i32, yRes);
    if (!vsync) WRITE_MEMORY (ASLR (0x1403D6189), u8, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90);
    if (unlockSongs) WRITE_MEMORY (ASLR (0x1401F6B78), u8, 0xB0, 0x01);

    // Bypass errors
    WRITE_MEMORY (ASLR (0x14001F0A0), u8, 0xC3);

    // Use TLS v1.2
    WRITE_MEMORY (ASLR (0x140309AC9), u8, 0x10);

    // Move various files to current dir
    WRITE_MEMORY (ASLR (0x140018351), u8, 0x02);
    WRITE_MEMORY (ASLR (0x1409A7138), char, "./"); // F:/
    WRITE_MEMORY (ASLR (0x1409D23B8), char, ".\\Setting1.bin");
    WRITE_MEMORY (ASLR (0x1409D23C8), char, ".\\Setting2.bin");

    // Remove datatable size limit
    {
        for (const auto address : memsetSizeAddresses)
            WRITE_MEMORY (ASLR (address) + 2, i32, datatableBufferSize);

        const auto bufferBase = MODULE_HANDLE - 0x01000000;
        AllocateStaticBufferNear ((void *)bufferBase, datatableBufferSize, &datatableBuffer);

        ReplaceLeaBufferAddress (datatableBufferAddresses, datatableBuffer.data ());
    }

    // Disable live check
    const auto amHandle = reinterpret_cast<u64> (GetModuleHandle ("AMFrameWork.dll"));
    INSTALL_HOOK_DYNAMIC (AMFWTerminate, reinterpret_cast<void *> (amHandle + 0x24B80));

    // Move various files to current directory
    WRITE_MEMORY (amHandle + 0x1473F, u8, 0xEB); // CreditLogPathA
    WRITE_MEMORY (amHandle + 0x148AA, u8, 0xEB); // CreditLogPathB
    WRITE_MEMORY (amHandle + 0x321A7, u8, 0xEB); // ErrorLogPathA
    WRITE_MEMORY (amHandle + 0x322FA, u8, 0xEB); // ErrorLogPathB
    WRITE_MEMORY (amHandle + 0x326D9, u8, 0xEB); // CommonLogPathA
    WRITE_MEMORY (amHandle + 0x3282C, u8, 0xEB); // CommonLogPathB
    WRITE_MEMORY (amHandle + 0x32C2A, u8, 0xEB); // BackupDataPathA
    WRITE_MEMORY (amHandle + 0x32D7D, u8, 0xEB); // BackupDataPathB
}
} // namespace patches::JPN00
