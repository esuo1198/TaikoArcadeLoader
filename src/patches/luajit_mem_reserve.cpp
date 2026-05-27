#include "helpers.h"
#include "patches.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cstdio>

namespace patches::LuaJITMem {
namespace {
HMODULE ntdll;
long (*avm) (HANDLE handle, void **addr, ULONG zbits, size_t *size, ULONG alloctype, ULONG prot);
void *reserved_mem;
size_t reserved_size               = 0;
volatile i64 lua_state             = 0;
SRWLOCK allocator_lock             = SRWLOCK_INIT;
thread_local bool gc_retrying      = false;

constexpr u32 BLOCK_MAGIC      = 0x54414C42;
constexpr u32 SLAB_MAGIC       = 0x54414C53;
constexpr size_t ALIGNMENT     = alignof (std::max_align_t);
constexpr size_t SLAB_SIZE     = 64 * 1024;
constexpr size_t BUDDY_MIN_SIZE = 2 * 1024;
constexpr size_t MAX_ORDERS    = 32;
constexpr size_t SMALL_MAX     = 4096;
constexpr size_t SMALL_FINE_MAX = 1024;
constexpr size_t SMALL_FINE_STEP = 16;
constexpr size_t SMALL_COARSE_STEP = 512;
constexpr size_t SMALL_FINE_CLASSES = SMALL_FINE_MAX / SMALL_FINE_STEP;
constexpr size_t SMALL_COARSE_CLASSES = (SMALL_MAX - SMALL_FINE_MAX) / SMALL_COARSE_STEP;
constexpr size_t SMALL_CLASSES = SMALL_FINE_CLASSES + SMALL_COARSE_CLASSES;
constexpr size_t LUA_STATE_SIZE = 5232;
constexpr size_t LARGE_BLOCK_SIZE = 1024 * 1024;

struct alignas (MEMORY_ALLOCATION_ALIGNMENT) BlockHeader {
    u32 magic;
    size_t size;
    u32 order;
    bool free;
    BlockHeader *prev;
    BlockHeader *next;
};

struct FreeSlot {
    FreeSlot *next;
};

struct alignas (MEMORY_ALLOCATION_ALIGNMENT) SlabHeader {
    u32 magic;
    u32 class_index;
    size_t slot_size;
    size_t total_slots;
    size_t free_count;
    FreeSlot *free_slots;
    SlabHeader *prev;
    SlabHeader *next;
};

struct AllocatorLock {
    AllocatorLock () { AcquireSRWLockExclusive (&allocator_lock); }
    ~AllocatorLock () { ReleaseSRWLockExclusive (&allocator_lock); }
};

constexpr size_t HEADER_SIZE      = (sizeof (BlockHeader) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
constexpr size_t SLAB_HEADER_SIZE = (sizeof (SlabHeader) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

BlockHeader *free_blocks[MAX_ORDERS];
SlabHeader *partial_slabs[SMALL_CLASSES];
size_t small_alloc_count[SMALL_CLASSES];
size_t small_free_count[SMALL_CLASSES];
size_t small_slab_count[SMALL_CLASSES];
size_t large_alloc_bytes = 0;
size_t large_free_bytes  = 0;
size_t protected_large_split_count = 0;

void write_allocator_dump (size_t failed_size);
size_t slot_size_for_class (size_t class_index);
size_t free_block_bytes ();

FUNCTION_PTR (i32, lua_gc, PROC_ADDRESS ("lua51.dll", "lua_gc"), i64, i32, i32);

size_t
align_size (size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

u32
order_for (size_t size) {
    size_t block_size = BUDDY_MIN_SIZE;
    for (u32 i = 0; i < MAX_ORDERS; i++) {
        if (size <= block_size) return i;
        block_size <<= 1;
    }
    return MAX_ORDERS;
}

size_t
block_size_for (u32 order) {
    return BUDDY_MIN_SIZE << order;
}

void
write_protected_split_log (size_t size) {
    LogMessage (LogLevel::DEBUG, "LuaJIT allocator split protected large block: count={} request={} free_bytes={} large_live={}", protected_large_split_count,
                size, free_block_bytes (), large_alloc_bytes - large_free_bytes);
}

void
write_gc_log (size_t size) {
    LogMessage (LogLevel::WARN, "LuaJIT allocator forced GC as last resort: request={} free_bytes={} large_live={}", size, free_block_bytes (),
                large_alloc_bytes - large_free_bytes);
}

void
push_free_block (BlockHeader *block) {
    block->magic = BLOCK_MAGIC;
    block->free = true;
    block->prev = nullptr;
    block->next = free_blocks[block->order];
    if (block->next) block->next->prev = block;
    free_blocks[block->order] = block;
}

void
remove_free_block (BlockHeader *block) {
    if (block->prev) block->prev->next = block->next;
    else free_blocks[block->order] = block->next;
    if (block->next) block->next->prev = block->prev;
    block->prev = nullptr;
    block->next = nullptr;
    block->free = false;
}

BlockHeader *
buddy_for (BlockHeader *block) {
    const auto offset       = static_cast<size_t> (reinterpret_cast<char *> (block) - reinterpret_cast<char *> (reserved_mem));
    const auto buddy_offset = offset ^ block_size_for (block->order);
    if (buddy_offset >= reserved_size) return nullptr;
    return reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (reserved_mem) + buddy_offset);
}

BlockHeader *
try_alloc_block (u32 target_order, size_t size, bool preserve_large_blocks = true) {
    if (target_order >= MAX_ORDERS) return nullptr;

    const auto large_order = order_for (LARGE_BLOCK_SIZE);
    const auto max_order   = preserve_large_blocks && target_order < large_order ? large_order : MAX_ORDERS;
    auto order = target_order;
    while (order < max_order && !free_blocks[order]) order++;
    if (order >= max_order) return nullptr;

    if (!preserve_large_blocks && target_order < large_order && order >= large_order) {
        protected_large_split_count++;
        write_protected_split_log (size);
    }

    auto block = free_blocks[order];
    remove_free_block (block);

    while (order > target_order) {
        order--;
        const auto half_size = block_size_for (order);
        auto buddy           = reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (block) + half_size);
        buddy->magic         = BLOCK_MAGIC;
        buddy->size          = 0;
        buddy->order         = order;
        buddy->prev          = nullptr;
        buddy->next          = nullptr;
        push_free_block (buddy);
        block->order = order;
    }

    block->size = size;
    block->free = false;
    if (size != SLAB_SIZE) large_alloc_bytes += block_size_for (block->order);
    return block;
}

BlockHeader *
alloc_block (u32 target_order, size_t size) {
    auto block = try_alloc_block (target_order, size);
    if (block) return block;

    if (target_order >= MAX_ORDERS) LogMessage (LogLevel::ERROR, "0x{:x} is too big for allocating on reserved space!", size);
    else LogMessage (LogLevel::ERROR, "Failed to allocate 0x{:x} on reserved space, consider increasing luajit_reserve_mem!", size);

    write_allocator_dump (size);
    ExitProcess (1);
}

void
free_block (BlockHeader *block) {
    while (block->order + 1 < MAX_ORDERS) {
        auto buddy = buddy_for (block);
        if (!buddy || buddy->magic != BLOCK_MAGIC || !buddy->free || buddy->order != block->order) break;

        remove_free_block (buddy);
        if (buddy < block) block = buddy;
        block->order++;
    }

    block->size  = 0;
    push_free_block (block);
}

size_t
free_block_bytes () {
    size_t result = 0;
    for (u32 order = 0; order < MAX_ORDERS; order++) {
        for (auto block = free_blocks[order]; block; block = block->next) result += block_size_for (order);
    }
    return result;
}

void
write_allocator_dump (size_t failed_size) {
    auto file = std::fopen ("luajit_allocator_dump.txt", "w");
    if (!file) return;

    const auto free_bytes = free_block_bytes ();
    std::fprintf (file, "failed_size=%zu\n", failed_size);
    std::fprintf (file, "reserved_size=%zu\n", reserved_size);
    std::fprintf (file, "buddy_free_bytes=%zu\n", free_bytes);
    std::fprintf (file, "large_alloc_bytes=%zu\n", large_alloc_bytes);
    std::fprintf (file, "large_free_bytes=%zu\n", large_free_bytes);
    std::fprintf (file, "large_live_bytes=%zu\n", large_alloc_bytes - large_free_bytes);

    std::fprintf (file, "\n[buddy_free_blocks]\n");
    for (u32 order = 0; order < MAX_ORDERS; order++) {
        size_t count = 0;
        for (auto block = free_blocks[order]; block; block = block->next) count++;
        if (count) std::fprintf (file, "order=%u block_size=%zu count=%zu bytes=%zu\n", order, block_size_for (order), count, count * block_size_for (order));
    }

    std::fprintf (file, "\n[small_classes]\n");
    for (size_t i = 0; i < SMALL_CLASSES; i++) {
        const auto live = small_alloc_count[i] - small_free_count[i];
        if (!live && !small_slab_count[i]) continue;

        size_t partial_count = 0;
        size_t partial_free_slots = 0;
        for (auto slab = partial_slabs[i]; slab; slab = slab->next) {
            partial_count++;
            partial_free_slots += slab->free_count;
        }

        std::fprintf (file, "slot_size=%zu live=%zu alloc=%zu free=%zu slabs=%zu partial_slabs=%zu partial_free_slots=%zu\n", slot_size_for_class (i),
                      live, small_alloc_count[i], small_free_count[i], small_slab_count[i], partial_count, partial_free_slots);
    }

    std::fclose (file);
}

size_t
class_index_for (size_t size) {
    size = std::max (size, sizeof (FreeSlot));
    if (size <= SMALL_FINE_MAX) return (size + SMALL_FINE_STEP - 1) / SMALL_FINE_STEP - 1;
    return SMALL_FINE_CLASSES + (size - SMALL_FINE_MAX + SMALL_COARSE_STEP - 1) / SMALL_COARSE_STEP - 1;
}

size_t
slot_size_for_class (size_t class_index) {
    if (class_index < SMALL_FINE_CLASSES) return (class_index + 1) * SMALL_FINE_STEP;
    return SMALL_FINE_MAX + (class_index - SMALL_FINE_CLASSES + 1) * SMALL_COARSE_STEP;
}

void
push_partial_slab (SlabHeader *slab) {
    slab->prev = nullptr;
    slab->next = partial_slabs[slab->class_index];
    if (slab->next) slab->next->prev = slab;
    partial_slabs[slab->class_index] = slab;
}

void
remove_partial_slab (SlabHeader *slab) {
    if (slab->prev) slab->prev->next = slab->next;
    else partial_slabs[slab->class_index] = slab->next;
    if (slab->next) slab->next->prev = slab->prev;
    slab->prev = nullptr;
    slab->next = nullptr;
}

SlabHeader *
slab_from_ptr (void *ptr) {
    const auto addr = reinterpret_cast<uintptr_t> (ptr);
    auto slab       = reinterpret_cast<SlabHeader *> (addr & ~(SLAB_SIZE - 1));
    return slab->magic == SLAB_MAGIC ? slab : nullptr;
}

void *
alloc_small (size_t size) {
    const auto class_index = class_index_for (size);
    auto slab              = partial_slabs[class_index];
    if (!slab) {
        slab             = reinterpret_cast<SlabHeader *> (alloc_block (order_for (SLAB_SIZE), SLAB_SIZE));
        slab->magic      = SLAB_MAGIC;
        slab->class_index = static_cast<u32> (class_index);
        slab->slot_size  = slot_size_for_class (class_index);
        slab->free_slots = nullptr;
        slab->prev       = nullptr;
        slab->next       = nullptr;

        const auto slot_start = reinterpret_cast<char *> (slab) + SLAB_HEADER_SIZE;
        slab->total_slots     = (SLAB_SIZE - SLAB_HEADER_SIZE) / slab->slot_size;
        slab->free_count      = slab->total_slots;
        small_slab_count[class_index]++;

        for (size_t i = 0; i < slab->total_slots; i++) {
            auto slot        = reinterpret_cast<FreeSlot *> (slot_start + i * slab->slot_size);
            slot->next       = slab->free_slots;
            slab->free_slots = slot;
        }

        push_partial_slab (slab);
    }

    auto slot        = slab->free_slots;
    slab->free_slots = slot->next;
    slab->free_count--;
    small_alloc_count[class_index]++;
    if (slab->free_count == 0) remove_partial_slab (slab);
    return slot;
}

void *
try_alloc_small (size_t size, bool preserve_large_blocks = true) {
    const auto class_index = class_index_for (size);
    auto slab              = partial_slabs[class_index];
    if (!slab) {
        slab = reinterpret_cast<SlabHeader *> (try_alloc_block (order_for (SLAB_SIZE), SLAB_SIZE, preserve_large_blocks));
        if (!slab) return nullptr;

        slab->magic       = SLAB_MAGIC;
        slab->class_index = static_cast<u32> (class_index);
        slab->slot_size   = slot_size_for_class (class_index);
        slab->free_slots  = nullptr;
        slab->prev        = nullptr;
        slab->next        = nullptr;

        const auto slot_start = reinterpret_cast<char *> (slab) + SLAB_HEADER_SIZE;
        slab->total_slots     = (SLAB_SIZE - SLAB_HEADER_SIZE) / slab->slot_size;
        slab->free_count      = slab->total_slots;
        small_slab_count[class_index]++;

        for (size_t i = 0; i < slab->total_slots; i++) {
            auto slot        = reinterpret_cast<FreeSlot *> (slot_start + i * slab->slot_size);
            slot->next       = slab->free_slots;
            slab->free_slots = slot;
        }

        push_partial_slab (slab);
    }

    auto slot        = slab->free_slots;
    slab->free_slots = slot->next;
    slab->free_count--;
    small_alloc_count[class_index]++;
    if (slab->free_count == 0) remove_partial_slab (slab);
    return slot;
}

void
free_small (SlabHeader *slab, void *ptr) {
    const auto was_full = slab->free_count == 0;
    auto slot           = reinterpret_cast<FreeSlot *> (ptr);
    slot->next          = slab->free_slots;
    slab->free_slots    = slot;
    slab->free_count++;
    small_free_count[slab->class_index]++;

    if (was_full) push_partial_slab (slab);
    if (slab->free_count != slab->total_slots) return;

    remove_partial_slab (slab);
    small_slab_count[slab->class_index]--;
    auto block   = reinterpret_cast<BlockHeader *> (slab);
    block->magic = BLOCK_MAGIC;
    block->order = order_for (SLAB_SIZE);
    free_block (block);
}

void *
alloc_from_reserved (size_t size) {
    if (size <= SMALL_MAX) return alloc_small (size);

    const auto block = alloc_block (order_for (HEADER_SIZE + size), size);
    return reinterpret_cast<char *> (block) + HEADER_SIZE;
}

void *
try_alloc_from_reserved (size_t size, bool preserve_large_blocks = true) {
    if (size <= SMALL_MAX) return try_alloc_small (size, preserve_large_blocks);

    const auto block = try_alloc_block (order_for (HEADER_SIZE + size), size, preserve_large_blocks);
    return block ? reinterpret_cast<char *> (block) + HEADER_SIZE : nullptr;
}

void
free_reserved (void *ptr) {
    if (auto slab = slab_from_ptr (ptr)) {
        free_small (slab, ptr);
        return;
    }

    auto block = reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (ptr) - HEADER_SIZE);
    large_free_bytes += block_size_for (block->order);
    free_block (block);
}

size_t
allocation_size (void *ptr) {
    if (auto slab = slab_from_ptr (ptr)) return slab->slot_size;
    auto block = reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (ptr) - HEADER_SIZE);
    return block->size;
}

bool
can_reuse (void *ptr, size_t size) {
    if (auto slab = slab_from_ptr (ptr)) return size <= slab->slot_size;

    auto block = reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (ptr) - HEADER_SIZE);
    return block_size_for (block->order) >= HEADER_SIZE + size;
}

void
collect_garbage_once (size_t size) {
    if (!lua_state || gc_retrying) return;

    write_gc_log (size);
    gc_retrying = true;
    lua_gc (lua_state, 2, 0);
    gc_retrying = false;
}

void *
lj_alloc_f (void *msp, void *ptr, size_t osize, size_t nsize) {
    (void)msp;
    (void)osize;

    if (!reserved_mem || !reserved_size) return nullptr;

    if (nsize == 0) {
        if (!ptr) return nullptr;
        AllocatorLock lock;
        free_reserved (ptr);
        return nullptr;
    }

    const auto size = align_size (nsize);

    if (!ptr) {
        AcquireSRWLockExclusive (&allocator_lock);
        auto result = try_alloc_from_reserved (size);
        ReleaseSRWLockExclusive (&allocator_lock);
        if (result) {
            if (!lua_state && nsize == LUA_STATE_SIZE) lua_state = reinterpret_cast<i64> (result);
            return result;
        }

        AcquireSRWLockExclusive (&allocator_lock);
        result = try_alloc_from_reserved (size, false);
        if (!result) {
            ReleaseSRWLockExclusive (&allocator_lock);
            collect_garbage_once (size);

            AcquireSRWLockExclusive (&allocator_lock);
            result = try_alloc_from_reserved (size, false);
        }
        if (!result) {
            LogMessage (LogLevel::ERROR, "Failed to allocate 0x{:x} on reserved space", size);
            write_allocator_dump (size);
            ReleaseSRWLockExclusive (&allocator_lock);
            ExitProcess (1);
        }
        ReleaseSRWLockExclusive (&allocator_lock);
        if (!lua_state && nsize == LUA_STATE_SIZE) lua_state = reinterpret_cast<i64> (result);
        return result;
    }

    AcquireSRWLockExclusive (&allocator_lock);

    if (can_reuse (ptr, size)) {
        if (!slab_from_ptr (ptr)) {
            auto block  = reinterpret_cast<BlockHeader *> (reinterpret_cast<char *> (ptr) - HEADER_SIZE);
            block->size = size;
        }
        ReleaseSRWLockExclusive (&allocator_lock);
        return ptr;
    }

    auto new_ptr = try_alloc_from_reserved (size);
    if (!new_ptr) {
        ReleaseSRWLockExclusive (&allocator_lock);

        AcquireSRWLockExclusive (&allocator_lock);
        new_ptr = try_alloc_from_reserved (size, false);
        if (!new_ptr) {
            ReleaseSRWLockExclusive (&allocator_lock);
            collect_garbage_once (size);

            AcquireSRWLockExclusive (&allocator_lock);
            new_ptr = try_alloc_from_reserved (size, false);
        }
        if (!new_ptr) {
            LogMessage (LogLevel::ERROR, "Failed to reallocate 0x{:x} on reserved space", size);
            write_allocator_dump (size);
            ReleaseSRWLockExclusive (&allocator_lock);
            ExitProcess (1);
        }
    }

    std::memcpy (new_ptr, ptr, std::min (allocation_size (ptr), size));
    free_reserved (ptr);
    ReleaseSRWLockExclusive (&allocator_lock);

    return new_ptr;
}

}

void
Init (size_t reserveSizeMB) {
    if (!reserveSizeMB) return;

    ntdll = GetModuleHandleA ("ntdll.dll");

    if (!ntdll) {
        LogMessage (LogLevel::ERROR, "Failed to get ntdll");
        return;
    }

    avm = reinterpret_cast<decltype (avm)> (GetProcAddress (ntdll, "NtAllocateVirtualMemory"));
    if (!avm) {
        LogMessage (LogLevel::ERROR, "Failed to get NtAllocateVirtualMemory");
        return;
    }

    reserved_mem  = nullptr;
    reserved_size = reserveSizeMB * 1024 * 1024;
    auto err      = avm (INVALID_HANDLE_VALUE, &reserved_mem, 1, &reserved_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (err != 0) {
        LogMessage (LogLevel::ERROR, "Failed to reserve low address memory for LuaJIT: {:x}", err);
        return;
    }

    LogMessage (LogLevel::INFO, "Reserved {}MB for LuaJIT at {}", reserved_size / 1024.f / 1024.f, reserved_mem);

    if (reserved_size <= HEADER_SIZE) {
        LogMessage (LogLevel::ERROR, "Reserved LuaJIT memory is too small");
        reserved_mem  = nullptr;
        reserved_size = 0;
        return;
    }

    for (auto &free_block : free_blocks) free_block = nullptr;
    for (auto &partial_slab : partial_slabs) partial_slab = nullptr;
    for (auto &alloc_count : small_alloc_count) alloc_count = 0;
    for (auto &free_count : small_free_count) free_count = 0;
    for (auto &slab_count : small_slab_count) slab_count = 0;
    large_alloc_bytes = 0;
    large_free_bytes  = 0;

    {
        auto remaining = reserved_size;
        auto addr      = reinterpret_cast<char *> (reserved_mem);
        while (remaining > 0) {
            size_t chunk = BUDDY_MIN_SIZE;
            while ((chunk << 1) <= remaining) chunk <<= 1;

            auto block   = reinterpret_cast<BlockHeader *> (addr);
            block->magic = BLOCK_MAGIC;
            block->size  = 0;
            block->order = order_for (chunk);
            block->prev  = nullptr;
            block->next  = nullptr;
            push_free_block (block);

            addr += chunk;
            remaining -= chunk;
        }
    }

    auto luajit = LoadLibraryA ("lua51.dll");
    assert (luajit);

    auto luaL_newstate_ptr = reinterpret_cast<uintptr_t> (GetProcAddress (luajit, "luaL_newstate"));
    assert (luaL_newstate_ptr);

    // don't call lj_alloc_create
    WRITE_NOP (luaL_newstate_ptr + 4, 5);
    WRITE_MEMORY (luaL_newstate_ptr + 0xC, u8, 0xEB);
    // use our own allocator
    WRITE_MEMORY (luaL_newstate_ptr + 0x13, u8, 0x48, 0xB9);
    WRITE_MEMORY (luaL_newstate_ptr + 0x15, void *, lj_alloc_f);
}

void
Exit () {
    if (reserved_mem) VirtualFree (reserved_mem, 0, MEM_RELEASE);
}
}
