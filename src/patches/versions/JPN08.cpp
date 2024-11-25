#include "helpers.h"
#include "../patches.h"
#include <safetyhook.hpp>

extern u64 song_data_size;
extern void *song_data;

#define RDX_MOV 0x48, 0xBA
#define R8_MOV  0x49, 0xB8
#define GENERATE_MOV(instruction, location)                                                                                 \
    instruction, (u8)(u64)(location), (u8)((u64)(location) >> 8), (u8)((u64)(location) >> 16), (u8)((u64)(location) >> 24), \
        (u8)((u64)(location) >> 32), (u8)((u64)(location) >> 40), (u8)((u64)(location) >> 48), (u8)((u64)(location) >> 56)

namespace patches::JPN08 {

HOOK_DYNAMIC (char, AMFWTerminate, i64) { return 0; }

const i32 datatableBufferSize = 1024 * 1024 * 12;
safetyhook::Allocation datatableBuffer;
const std::vector<uintptr_t> datatableBufferAddresses
    = {0x14006D9A6, 0x14006D9D3, 0x14006E048, 0x14006E075, 0x14006E3A8, 0x14006E3D5, 0x14006E988, 0x14006E9B5, 0x14006EE22, 0x14006EE51, 0x14006F068,
       0x14006F095, 0x14006F2F8, 0x14006F325, 0x14006F698, 0x14006F6C5, 0x14006F919, 0x14006F948, 0x14006FC38, 0x14006FC67, 0x14007006C, 0x140070099,
       0x1400703E3, 0x140070412, 0x140070EB3, 0x140070EE2, 0x140071748, 0x140071775, 0x140071A68, 0x140071A95, 0x140071DD2, 0x140071E04, 0x140072E44,
       0x140072E73, 0x140073058, 0x140073085, 0x140073374, 0x1400733A0, 0x1400735E8, 0x140073615, 0x14007390C, 0x140073939, 0x140073E73, 0x140073EA6,
       0x140074A8D, 0x140074ABC, 0x140075082, 0x1400750B1, 0x140075524, 0x140075550, 0x1400758A2, 0x1400758D1, 0x140075D88, 0x140075DB5, 0x1403BA8FD};
const std::vector<uintptr_t> memsetSizeAddresses
    = {0x14006D9A0, 0x14006E042, 0x14006E3A2, 0x14006E982, 0x14006EE1C, 0x14006F062, 0x14006F2F2, 0x14006F692, 0x14006F913,
       0x14006FC32, 0x140070066, 0x1400703DD, 0x140070EAD, 0x140071742, 0x140071A62, 0x140071DCC, 0x140072E3E, 0x140073052,
       0x14007336E, 0x1400735E2, 0x140073906, 0x140073E6D, 0x140074A87, 0x14007507C, 0x14007551E, 0x14007589C, 0x140075D82};

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
    LogMessage (LOG_LEVEL_INFO, "Init JPN08 patches");
    i32 xRes         = 1920;
    i32 yRes         = 1080;
    bool vsync       = false;
    bool unlockSongs = true;

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        auto patches = openConfigSection (config_ptr.get (), "patches");
        if (patches) unlockSongs = readConfigBool (patches, "unlock_songs", unlockSongs);

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
    WRITE_MEMORY (ASLR (0x14035FC5B), i32, xRes);
    WRITE_MEMORY (ASLR (0x14035FC62), i32, yRes);
    if (!vsync) WRITE_MEMORY (ASLR (0x140517339), u8, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90);
    if (unlockSongs) WRITE_MEMORY (ASLR (0x140314E8D), u8, 0xB0, 0x01);

    // Bypass errors
    WRITE_MEMORY (ASLR (0x1400239C0), u8, 0xC3);

    // Use TLS v1.2
    WRITE_MEMORY (ASLR (0x14044B1A9), u8, 0x10);

    // Move various files to current directory
    WRITE_MEMORY (ASLR (0x14001C941), u8, 0x02);
    WRITE_MEMORY (ASLR (0x140B1B4B0), char, "./"); // F:/
    WRITE_MEMORY (ASLR (0x140B5C528), char, ".\\Setting1.bin");
    WRITE_MEMORY (ASLR (0x140B5C538), char, ".\\Setting2.bin");

    // Remove datatable size limit
    {
        for (auto address : memsetSizeAddresses)
            WRITE_MEMORY (ASLR (address) + 2, i32, datatableBufferSize);

        auto bufferBase = MODULE_HANDLE - 0x01000000;
        AllocateStaticBufferNear ((void *)bufferBase, datatableBufferSize, &datatableBuffer);

        ReplaceLeaBufferAddress (datatableBufferAddresses, datatableBuffer.data ());
    }

    // Remove song limit
    {
        WRITE_MEMORY (ASLR (0x140313726), i32, 9000);
        WRITE_MEMORY (ASLR (0x1402F39E6), i32, 9000);
        WRITE_MEMORY (ASLR (0x1402F3AB0), i32, 9000);
        WRITE_MEMORY (ASLR (0x1402F3BE4), i32, 9000);
        WRITE_MEMORY (ASLR (0x14030643B), i32, 9000);
        WRITE_MEMORY (ASLR (0x140306507), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403065D3), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403066FB), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403067C7), i32, 9000);
        WRITE_MEMORY (ASLR (0x140306893), i32, 9000);
        WRITE_MEMORY (ASLR (0x14030698B), i32, 9000);
        WRITE_MEMORY (ASLR (0x140313666), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403139F4), i32, 9000);
        WRITE_MEMORY (ASLR (0x140313B04), i32, 9000);
        WRITE_MEMORY (ASLR (0x140313C24), i32, 9000);
        WRITE_MEMORY (ASLR (0x140313CF4), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403140C4), i32, 9000);
        WRITE_MEMORY (ASLR (0x1403147AA), i32, 9000);
        WRITE_MEMORY (ASLR (0x140225FB6), i32, 9000);
        WRITE_MEMORY (ASLR (0x140226146), i32, 9000);
        WRITE_MEMORY (ASLR (0x140314DCC), i32, 9000);
        WRITE_MEMORY (ASLR (0x140314EC9), i32, 9000);
        WRITE_MEMORY (ASLR (0x140338E2C), i32, 9000);
        WRITE_MEMORY (ASLR (0x1400EE0A4), i32, 9000);
        WRITE_MEMORY (ASLR (0x1400EE8B5), i32, 9000);
        WRITE_MEMORY (ASLR (0x1400EEDA6), i32, 9000);
        WRITE_MEMORY (ASLR (0x140315608), i32, 9000);
        WRITE_MEMORY (ASLR (0x14034A7EB), i32, 9000);
        WRITE_MEMORY (ASLR (0x1402F3CB3), i32, 9000);
        WRITE_MEMORY (ASLR (0x140314059), i32, 9000);
        WRITE_MEMORY (ASLR (0x140226063), i32, 9000);
        WRITE_MEMORY (ASLR (0x14022609F), i32, 9000);
        WRITE_MEMORY (ASLR (0x140226296), i32, 9000);
        WRITE_MEMORY (ASLR (0x140306A2E), i32, 9000);
        WRITE_MEMORY (ASLR (0x140314F46), i32, 9000);
        WRITE_MEMORY (ASLR (0x140314F97), i32, 9000);

        song_data = malloc (song_data_size);
        memset (song_data, 0, song_data_size);

        // Song data
        WRITE_MEMORY (ASLR (0x14031367B), u8, GENERATE_MOV (R8_MOV, song_data));
        // Crown data
        WRITE_MEMORY (ASLR (0x1402F3AC6), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1402F39FC), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1402F3BFA), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1403140D7), u8, GENERATE_MOV (R8_MOV, song_data));
        // Score ranks
        WRITE_MEMORY (ASLR (0x1403065EA), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x14030651E), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140306452), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1403068AA), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1403067DE), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140306712), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x1403069A2), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_NOP (ASLR (0x1403069AC), 0x05);
        // Unknown
        WRITE_MEMORY (ASLR (0x140313755), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140313A0B), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140313B4C), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140313D38), u8, GENERATE_MOV (RDX_MOV, song_data));
        WRITE_MEMORY (ASLR (0x140313C42), u8, GENERATE_MOV (R8_MOV, song_data));
    }

    // Disable live check
    auto amHandle = (u64)GetModuleHandle ("AMFrameWork.dll");
    INSTALL_HOOK_DYNAMIC (AMFWTerminate, (void *)(amHandle + 0x35A00));

    // Move various files to current directory
    WRITE_MEMORY (amHandle + 0x148AF, u8, 0xEB); // CreditLogPathA
    WRITE_MEMORY (amHandle + 0x14A1A, u8, 0xEB); // CreditLogPathB
    WRITE_MEMORY (amHandle + 0x33EF7, u8, 0xEB); // ErrorLogPathA
    WRITE_MEMORY (amHandle + 0x3404A, u8, 0xEB); // ErrorLogPathB
    WRITE_MEMORY (amHandle + 0x34429, u8, 0xEB); // CommonLogPathA
    WRITE_MEMORY (amHandle + 0x3457C, u8, 0xEB); // CommonLogPathB
    WRITE_MEMORY (amHandle + 0x3497A, u8, 0xEB); // BackupDataPathA
    WRITE_MEMORY (amHandle + 0x34ACD, u8, 0xEB); // BackupDataPathB
}
} // namespace patches::JPN08
