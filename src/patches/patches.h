#pragma once
#include <string>
#include <functional>
#include <pugixml.hpp>

#include "constants.h"

namespace patches {
namespace JPN00 {
void Init ();
} // namespace JPN00
namespace JPN08 {
void Init ();
} // namespace JPN08
namespace JPN39 {
void Init ();
} // namespace JPN39
namespace CHN00 {
void Init ();
} // namespace CHN00
namespace Dxgi {
void Init ();
} // namespace Dxgi
namespace FpsLimiter {
void Init   (float fpsLimit);
void Update ();
} // namespace FpsLimiter
namespace Audio {
void Init          ();
void SetVolumeRate (float rate);
} // namespace Audio
namespace Qr {
void Init   ();
void Update ();
} // namespace Qr
namespace AmAuth {
void Init ();
} // namespace AmAuth
namespace LayeredFs {
void Init ();
void RegisterBefore (const std::function<std::string (const std::string, const std::string)> &fileHandler);
void RegisterAfter  (const std::function<std::string (const std::string, const std::string)> &fileHandler);
} // namespace LayeredFs
namespace TestMode {
class Applicable {
public:
    virtual pugi::xml_node *Apply (pugi::xml_node *doc, pugi::xml_node *node) = 0;
};
class Menu : public Applicable {
public:
    virtual void RegisterItem (Applicable *item) = 0;
};
typedef u64 (*RefTestModeMain) (u64);
void Init              ();
void Append            (const pugi::xml_node &node, const wchar_t *attr, const std::wstring &append);
void SetupAccessor     (u64 appAccessor, RefTestModeMain refTestMode);
int  ReadTestModeValue (const wchar_t *itemId);
Menu *CreateMenu       (const std::wstring &menuName, const std::wstring &menuId);
void RegisterItem      (const std::wstring &item, const std::function<void ()> &initMethod, Menu *menu);
void RegisterItem      (const std::wstring &item, const std::function<void ()> &initMethod);
void RegisterItem      (const std::wstring &item, Menu *menu);
void RegisterItem      (const std::wstring &item);
void RegisterItem      (Applicable *item, Menu *menu);
void RegisterItem      (Applicable *item);
void RegisterItemAfter (const std::wstring &query, const std::wstring &item, const std::function<void()> &initMethod);
void RegisterItemAfter (const std::wstring &query, const std::wstring &item);
void RegisterItemAfter (const std::wstring &query, Applicable *item);
void RegisterModify    (const std::wstring &query, const std::function<void (pugi::xml_node &)> &nodeModify, const std::function<void ()> &initMethod);
void RegisterModify    (const std::wstring &query, const std::function<void (pugi::xml_node &)> &nodeModify);
} // namespace TestMode
namespace Plugins {
// typedefs
typedef void (*CallBackTouchCard)     (int32_t, int32_t, uint8_t[168], uint64_t);
typedef bool (*CommitCardCallback)    (std::string, std::string);
typedef bool (*CommitQrCallback)      (std::vector<uint8_t> &);
typedef bool (*CommitQrLoginCallback) (std::string);
// Standard API
void Init           ();
void Update         ();
void Exit           ();
// Lowlevel Card API
void WaitTouch      (CallBackTouchCard callback, uint64_t touchData);
// Lowlevel QR API
void   InitQr       (GameVersion gameVersion);
void   UsingQr      ();
void * CheckQr      ();
size_t GetQr        (void *plugin, size_t size, uint8_t *buffer);
// New API
void InitVersion    (GameVersion gameVersion);
void InitCardReader (CommitCardCallback touch);
void InitQRScanner  (CommitQrCallback scan);
void InitQRLogin    (CommitQrLoginCallback login);
void UpdateStatus   (size_t type, bool status);
// Plugins Loader
void LoadPlugins    ();
} // namespace Plugins
namespace Scanner {
enum class State { Disable, Ready, CopyWait };
void Init        ();
void Update      ();
namespace Card {
typedef int32_t (*CallbackAttach) (int32_t, int32_t, int32_t *);
typedef void    (*CallbackTouch)  (int32_t, int32_t, uint8_t[168], uint64_t);
void Init        ();
void Update      ();
bool Commit      (std::string accessCode, std::string chipId);
} // namespace Card
namespace Qr {
void Init        ();
void Update      ();
bool Commit      (std::vector<uint8_t> &buffer);
bool CommitLogin (std::string accessCode);
std::vector<uint8_t> &ReadQRData  (std::vector<uint8_t> &buffer);
std::vector<uint8_t> &ReadQRImage (std::vector<uint8_t> &buffer);
} // namespace Qr
} // namespace Scanner
namespace LuaJITMem {
void Init              (size_t reserveSizeMB);
void Exit              ();
}
} // namespace patches
