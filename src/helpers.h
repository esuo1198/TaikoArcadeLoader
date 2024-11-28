#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
// ReSharper disable once CppUnusedIncludeDirective
#include <safetyhook.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <MinHook.h>
#include <string>
#include <toml.h>
#include <windows.h>
#include "constants.h"
#include "logger.h"

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define FUNCTION_PTR(returnType, function, location, ...) returnType (*function) (__VA_ARGS__) = (returnType (*) (__VA_ARGS__)) (location)
#define FUNCTION_PTR_H(returnType, function, ...)         extern returnType (*function) (__VA_ARGS__)

#define PROC_ADDRESS(libraryName, procName)      GetProcAddress (LoadLibrary (TEXT (libraryName)), procName)
#define PROC_ADDRESS_OFFSET(libraryName, offset) (u64) ((u64)GetModuleHandle (TEXT (libraryName)) + offset)

#define BASE_ADDRESS 0x140000000
#ifdef BASE_ADDRESS
const HMODULE MODULE_HANDLE = GetModuleHandle (nullptr);
#define ASLR(address) ((u64)MODULE_HANDLE + (u64)address - (u64)BASE_ADDRESS)
#endif

#define HOOK(returnType, functionName, location, ...)         \
    typedef returnType (*functionName) (__VA_ARGS__);         \
    functionName original##functionName = nullptr;            \
    void *where##functionName           = (void *)(location); \
    returnType implOf##functionName (__VA_ARGS__)

#define HOOK_DYNAMIC(returnType, functionName, ...)   \
    typedef returnType (*functionName) (__VA_ARGS__); \
    functionName original##functionName = nullptr;    \
    void *where##functionName           = nullptr;    \
    returnType implOf##functionName (__VA_ARGS__)

#define VTABLE_HOOK(returnType, className, functionName, ...)                      \
    typedef returnType (*className##functionName) (className * This, __VA_ARGS__); \
    className##functionName original##className##functionName = nullptr;           \
    void *where##className##functionName                      = nullptr;           \
    returnType implOf##className##functionName (className *This, __VA_ARGS__)

#define MID_HOOK(functionName, location, ...)   \
    typedef void (*functionName) (__VA_ARGS__); \
    SafetyHookMid midHook##functionName{};      \
    u64 where##functionName = (location);       \
    void implOf##functionName (SafetyHookContext &ctx)

#define MID_HOOK_DYNAMIC(functionName, ...)           \
    typedef void (*functionName) (__VA_ARGS__);       \
    std::map<u64, SafetyHookMid> mapOf##functionName; \
    void implOf##functionName (SafetyHookContext &ctx)

#define INSTALL_HOOK(functionName)                                                                                     \
    {                                                                                                                  \
        LogMessage (LogLevel::DEBUG, std::string ("Installing hook for ") + #functionName);                            \
        MH_Initialize ();                                                                                              \
        MH_CreateHook ((void *)where##functionName, (void *)implOf##functionName, (void **)(&original##functionName)); \
        MH_EnableHook ((void *)where##functionName);                                                                   \
    }

#define INSTALL_HOOK_DYNAMIC(functionName, location) \
    {                                                \
        where##functionName = (void *)location;      \
        INSTALL_HOOK (functionName);                 \
    }

#define INSTALL_HOOK_DIRECT(location, locationOfHook)                                          \
    {                                                                                          \
        LogMessage (LogLevel::DEBUG, std::string ("Installing direct hook for ") + #location); \
        MH_Initialize ();                                                                      \
        MH_CreateHook ((void *)(location), (void *)(locationOfHook), NULL);                    \
        MH_EnableHook ((void *)(location));                                                    \
    }

#define INSTALL_VTABLE_HOOK(className, object, functionName, functionIndex)                     \
    {                                                                                           \
        where##className##functionName = (*(className##functionName ***)object)[functionIndex]; \
        INSTALL_HOOK (className##functionName);                                                 \
    }

#define INSTALL_MID_HOOK(functionName)                                                              \
    {                                                                                               \
        LogMessage (LogLevel::DEBUG, std::string ("Installing mid hook for ") + #functionName);     \
        midHook##functionName = safetyhook::create_mid (where##functionName, implOf##functionName); \
    }

#define INSTALL_MID_HOOK_DYNAMIC(functionName, location) \
    { mapOf##functionName[location] = safetyhook::create_mid (location, implOf##functionName); }

inline bool sendFlag = false;
#define SCENE_RESULT_HOOK(functionName, location)                                                                                \
    HOOK (void, functionName, location, i64 a1, i64 a2, i64 a3) {                                                                \
        if (TestMode::ReadTestModeValue (L"ModInstantResult") != 1 && TestMode::ReadTestModeValue (L"NumberOfStageItem") <= 4) { \
            original##functionName (a1, a2, a3);                                                                                 \
            return;                                                                                                              \
        }                                                                                                                        \
        sendFlag = true;                                                                                                         \
        original##functionName (a1, a2, a3);                                                                                     \
        ExecuteSendResultData ();                                                                                                \
    }

#define SEND_RESULT_HOOK(functionName, location)                                                                                 \
    HOOK (void, functionName, location, i64 a1) {                                                                                \
        if (TestMode::ReadTestModeValue (L"ModInstantResult") != 1 && TestMode::ReadTestModeValue (L"NumberOfStageItem") <= 4) { \
            original##functionName (a1);                                                                                         \
            return;                                                                                                              \
        }                                                                                                                        \
        if (sendFlag) {                                                                                                          \
            sendFlag = false;                                                                                                    \
            original##functionName (a1);                                                                                         \
        }                                                                                                                        \
    }

#define CHANGE_RESULT_SIZE_HOOK(functionName, location, target)                                                                            \
    MID_HOOK (functionName, location, SafetyHookContext &ctx) {                                                                            \
        if (TestMode::ReadTestModeValue (L"ModInstantResult") != 1 && TestMode::ReadTestModeValue (L"NumberOfStageItem") <= 4) { return; } \
        i64 instance          = RefPlayDataManager (*(i64 *)ctx.r12);                                                                      \
        u32 currentStageCount = *(u32 *)(instance + 8);                                                                                    \
        ctx.target &= 0xFFFFFFFF00000000;                                                                                                  \
        ctx.target |= currentStageCount;                                                                                                   \
    }

#define CHANGE_RESULT_INDEX_HOOK(functionName, location, target, offset, skip)                                                             \
    MID_HOOK (functionName, location, SafetyHookContext &ctx) {                                                                            \
        if (TestMode::ReadTestModeValue (L"ModInstantResult") != 1 && TestMode::ReadTestModeValue (L"NumberOfStageItem") <= 4) { return; } \
        i64 instance          = RefPlayDataManager (*(i64 *)ctx.r12);                                                                      \
        u32 currentStageCount = *(u32 *)(instance + 8);                                                                                    \
        ctx.target &= 0xFFFFFFFF00000000;                                                                                                  \
        ctx.target |= currentStageCount - 1;                                                                                               \
        *(u32 *)(ctx.rsp + offset) = currentStageCount - 1;                                                                                \
        ctx.rip += skip;                                                                                                                   \
    }

#define READ_MEMORY(location, type) *(type *)location

#define WRITE_MEMORY(location, type, ...)                                                        \
    {                                                                                            \
        const type data[] = {__VA_ARGS__};                                                       \
        DWORD oldProtect;                                                                        \
        VirtualProtect ((void *)(location), sizeof (data), PAGE_EXECUTE_READWRITE, &oldProtect); \
        memcpy ((void *)(location), data, sizeof (data));                                        \
        VirtualProtect ((void *)(location), sizeof (data), oldProtect, &oldProtect);             \
    }

#define WRITE_MEMORY_STRING(location, data, length)                                       \
    {                                                                                     \
        DWORD oldProtect;                                                                 \
        VirtualProtect ((void *)(location), length, PAGE_EXECUTE_READWRITE, &oldProtect); \
        memcpy ((void *)(location), data, length);                                        \
        VirtualProtect ((void *)(location), length, oldProtect, &oldProtect);             \
    }

#define WRITE_NOP(location, count)                                                                 \
    {                                                                                              \
        DWORD oldProtect;                                                                          \
        VirtualProtect ((void *)(location), (size_t)(count), PAGE_EXECUTE_READWRITE, &oldProtect); \
        for (size_t i = 0; i < (size_t)(count); i++)                                               \
            *((u8 *)(location) + i) = 0x90;                                                        \
        VirtualProtect ((void *)(location), (size_t)(count), oldProtect, &oldProtect);             \
    }

#define WRITE_NULL(location, count)                                                                \
    {                                                                                              \
        DWORD oldProtect;                                                                          \
        VirtualProtect ((void *)(location), (size_t)(count), PAGE_EXECUTE_READWRITE, &oldProtect); \
        for (size_t i = 0; i < (size_t)(count); i++)                                               \
            *((u8 *)(location) + i) = 0x00;                                                        \
        VirtualProtect ((void *)(location), (size_t)(count), oldProtect, &oldProtect);             \
    }

#define round(num) ((num > 0) ? (int)(num + 0.5) : (int)(num - 0.5))

toml_table_t *openConfig (const std::filesystem::path &path);
toml_table_t *openConfigSection (const toml_table_t *config, const std::string &sectionName);
bool readConfigBool (const toml_table_t *table, const std::string &key, bool notFoundValue);
i64 readConfigInt (const toml_table_t *table, const std::string &key, i64 notFoundValue);
std::string readConfigString (const toml_table_t *table, const std::string &key, const std::string &notFoundValue);
std::vector<i64> readConfigIntArray (const toml_table_t *table, const std::string &key, std::vector<i64> notFoundValue);
std::wstring replace (const std::wstring &orignStr, const std::wstring &oldStr, const std::wstring &newStr);
std::string replace (const std::string &orignStr, const std::string &oldStr, const std::string &newStr);
const char *GameVersionToString (GameVersion version);
const char *languageStr (int language);
std::string ConvertWideToUtf8 (const std::wstring &wstr);
bool AreAllBytesZero (const u8 *array, size_t offset, size_t length);
