#pragma once
#include <string>
#include <functional>
#include <pugixml.hpp>

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
void Init (float fpsLimit);
void Update ();
} // namespace FpsLimiter
namespace Audio {
void Init ();
} // namespace Audio
namespace Qr {
void Init ();
void Update ();
} // namespace Qr
namespace AmAuth {
void Init ();
} // namespace AmAuth
namespace LayeredFs {
void Init ();
void RegisterBefore (const std::function<std::string (const std::string, const std::string)> &fileHandler);
void RegisterAfter (const std::function<std::string (const std::string, const std::string)> &fileHandler);
} // namespace LayeredFs
namespace TestMode {
typedef u64 (*RefTestModeMain) (u64);
void Init ();
void SetupAccessor (u64 appAccessor, RefTestModeMain refTestMode);
int ReadTestModeValue (const wchar_t *itemId);
void RegisterItem (const std::wstring& item, const std::function<void ()> &initMethod);
void RegisterModify (const std::wstring& query, const std::function<void (pugi::xml_node &)> &nodeModify, const std::function<void ()> &initMethod);
void Append (const pugi::xml_node &node, const wchar_t *attr, const std::wstring& append);
} // namespace TestMode
} // namespace patches
