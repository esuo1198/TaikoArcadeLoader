#include "constants.h"
#include "helpers.h"
#include "patches.h"
#include <exception>

extern GameVersion gameVersion;

bool chsPatch = false;

namespace patches::TestMode {
class RegisteredItem {
public:
    std::wstring selectItem;
    std::function<void ()> registerInit;
    RegisteredItem (const std::wstring &selectItem, const std::function<void ()> &initMethod) {
        this->selectItem   = selectItem;
        this->registerInit = initMethod;
    }
};
class RegisteredModify {
public:
    std::wstring query;
    std::function<void (pugi::xml_node &)> nodeModify;
    std::function<void ()> registerInit;
    RegisteredModify (const std::wstring &query, const std::function<void (pugi::xml_node &)> &nodeModify, const std::function<void ()> &initMethod) {
        this->query        = query;
        this->nodeModify   = nodeModify;
        this->registerInit = initMethod;
    }
};

std::vector<RegisteredItem *> registeredItems      = {};
std::vector<RegisteredModify *> registeredModifies = {};
std::wstring moddedInitial                         = L"";
std::wstring modded                                = L"";

u64 appAccessor             = 0;
RefTestModeMain refTestMode = nullptr;

pugi::xml_document &
CreateMenu (pugi::xml_document &menuMain, std::wstring menuId, std::wstring menuName, std::vector<std::wstring> items, std::wstring backId) {
    LogMessage (LogLevel::DEBUG, L"Create Menu " + menuName);
    std::wstring menuBasicLine = L"<menu id=\"" + menuId + L"\"></menu>";
    if (menuMain.load_string (menuBasicLine.c_str ())) {
        pugi::xml_node menu       = menuMain.first_child ();
        pugi::xml_node menuHeader = menu.append_child (L"layout");
        pugi::xml_node menuCenter = menu.append_child (L"layout");
        pugi::xml_node menuFooter = menu.append_child (L"layout");
        // Mod Manager Menu Header
        menuHeader.append_attribute (L"type") = L"Header";
        menuHeader.append_child (L"break-item");
        pugi::xml_node menuTitle              = menuHeader.append_child (L"text-item");
        std::wstring menuNameFull             = L"     " + menuName;
        menuTitle.append_attribute (L"label") = menuNameFull.c_str ();
        menuHeader.append_child (L"break-item");
        menuHeader.append_child (L"break-item");
        pugi::xml_node menuTitleData                       = menuHeader.append_child (L"data-item");
        menuTitleData.append_attribute (L"data")           = L"HeaderTextData";
        menuTitleData.append_attribute (L"param-offset-x") = L"-15";
        menuHeader.append_child (L"break-item");
        // Mod Manager Menu Center
        menuCenter.append_attribute (L"type")      = L"Center";
        menuCenter.append_attribute (L"padding-x") = L"23";
        for (std::wstring item : items) {
            std::wstring itemLine = L"<root>" + item + L"</root>";
            if (pugi::xml_document menuItem; menuItem.load_string (itemLine.c_str ())) menuCenter.append_copy (menuItem.first_child ().first_child ());
            else LogMessage (LogLevel::ERROR, L"Failed to parse option line: " + item);
            menuCenter.append_child (L"break-item");
        }
        menuCenter.append_child (L"break-item");
        pugi::xml_node menuCenterExit              = menuCenter.append_child (L"menu-item");
        menuCenterExit.append_attribute (L"label") = L"EXIT";
        menuCenterExit.append_attribute (L"menu")  = backId.c_str ();
        // Mod Manager Menu Footer
        menuFooter.append_attribute (L"type")               = L"Footer";
        pugi::xml_node menuFooterData                       = menuFooter.append_child (L"data-item");
        menuFooterData.append_attribute (L"data")           = L"FooterTextData";
        menuFooterData.append_attribute (L"param-offset-x") = L"-20";
    }
    return menuMain;
}

std::wstring
ReadXMLFileSwitcher (std::wstring &fileName) {
    const std::size_t pos   = fileName.rfind (L"/");
    std::wstring base = fileName.substr (0, pos + 1);
    const std::wstring file = fileName.substr (pos + 1, fileName.size ());

    if (gameVersion == GameVersion::JPN39 && chsPatch) {
        if (file.starts_with (L"DeviceInitialize")) base.append (L"DeviceInitialize_china.xml");
        if (file.starts_with (L"TestMode")) base.append (L"TestMode_china.xml");
        if (std::filesystem::exists (base)) return base;
    }

    return fileName;
}

HOOK_DYNAMIC (void, TestModeSetMenuHook, u64 testModeLibrary, const wchar_t *lFileName) {
    auto originalFileName = std::wstring (lFileName);
    std::wstring fileName         = originalFileName;
    if (fileName.ends_with (L"DeviceInitialize.xml") || fileName.ends_with (L"DeviceInitialize_asia.xml")
        || fileName.ends_with (L"DeviceInitialize_china.xml")) {
        if (moddedInitial == L"") {
            fileName = ReadXMLFileSwitcher (fileName);
            if (pugi::xml_document doc; !doc.load_file (fileName.c_str ())) {
                LogMessage (LogLevel::ERROR, L"Loading DeviceInitialize structure failed! path: " + fileName);
                moddedInitial = fileName;
            } else {
                std::wstring modFileName
                    = replace (replace (replace (fileName, L"lize_asia.xml", L"lize_mod.xml"), L"lize_china.xml", L"lize_mod.xml"), L"lize.xml",
                               L"lize_mod.xml");
                auto dongleQuery = pugi::xpath_query (L"/root/menu[@id='TopMenu']/layout[@type='Center']/select-item[@id='DongleItem']");
                pugi::xml_node dongleItem     = doc.select_node (dongleQuery).node ();
                pugi::xml_node talItem        = dongleItem.parent ().append_copy (dongleItem);
                talItem.attribute (L"label").set_value (L"TAIKOARCADELOADER");
                talItem.attribute (L"id").set_value (L"TaikoArcadeLoader");
                talItem.append_attribute (L"default") = L"1";
                dongleItem.parent ().append_child (L"break-item");

                [[maybe_unused]] auto ignored = doc.save_file (modFileName.c_str ());
                moddedInitial = modFileName;
                fileName      = modFileName;
            }
        } else fileName = moddedInitial;
    } else if (fileName.ends_with (L"TestMode.xml") || fileName.ends_with (L"TestMode_asia.xml") || fileName.ends_with (L"TestMode_china.xml")) {
        if (modded == L"") {
            if (!registeredItems.empty () || !registeredModifies.empty ()) {
                fileName = ReadXMLFileSwitcher (fileName);
                if (pugi::xml_document doc; !doc.load_file (fileName.c_str ())) {
                    LogMessage (LogLevel::ERROR, L"Loading TestMode structure failed! path: " + fileName);
                    modded = fileName;
                } else {
                    std::wstring modFileName
                        = replace (replace (replace (fileName, L"Mode_asia.xml", L"Mode_mod.xml"), L"Mode_china.xml", L"Mode_mod.xml"), L"Mode.xml",
                                   L"Mode_mod.xml");
                    if (!registeredItems.empty ()) { auto menuQuery
                            = pugi::xpath_query (L"/root/menu[@id='TopMenu']/layout[@type='Center']/menu-item[@menu='GameOptionsMenu']");
                        pugi::xml_node menuItem                  = doc.select_node (menuQuery).node ();
                        menuItem                                 = menuItem.next_sibling ();
                        pugi::xml_node modMenuEntry              = menuItem.parent ().insert_child_after (L"menu-item", menuItem);
                        modMenuEntry.append_attribute (L"label") = L"MOD MANAGER";
                        modMenuEntry.append_attribute (L"menu")  = L"ModManagerMenu";
                        menuItem.parent ().insert_child_after (L"break-item", modMenuEntry);

                        pugi::xml_document modMenu;
                        std::vector<std::wstring> toInsertItems = {};
                        for (RegisteredItem *item : registeredItems) {
                            toInsertItems.push_back (item->selectItem);
                            item->registerInit ();
                        }
                        CreateMenu (modMenu, L"ModManagerMenu", L"MOD MANAGER", toInsertItems, L"TopMenu");
                        auto topMenuQuery = pugi::xpath_query (L"/root/menu[@id='TopMenu']");
                        pugi::xml_node topMenu         = doc.select_node (topMenuQuery).node ();
                        topMenu.parent ().insert_copy_after (modMenu.first_child (), topMenu);
                    }

                    if (!registeredModifies.empty ()) {
                        for (RegisteredModify *modify : registeredModifies) { auto modifyQuery = pugi::xpath_query (modify->query.c_str ());
                            try {
                                if (pugi::xml_node modifyNode = doc.select_node (modifyQuery).node ()) {
                                    modify->nodeModify (modifyNode);
                                    modify->registerInit ();
                                }
                            } catch ([[maybe_unused]] std::exception &e) { LogMessage (LogLevel::ERROR, L"Failed to find node by xpath: " + modify->query); }
                        }
                    }

                    [[maybe_unused]] auto _ = doc.save_file (modFileName.c_str ());
                    modded   = modFileName;
                    fileName = modFileName;
                }
            } else modded = fileName;
        } else fileName = modded;
    }

    LogMessage (LogLevel::DEBUG, L"TestModeLibrary load: " + fileName);
    originalTestModeSetMenuHook (testModeLibrary, fileName.c_str ());
}

void
CommonModify () {
    // Default off Close time
    TestMode::RegisterModify (
        L"/root/menu[@id='CloseTimeSettingMenu']/layout[@type='Center']/select-item[@id='ScheduleTypeItem']",
        [&] (const pugi::xml_node &node) { node.attribute (L"default").set_value (L"0"); }, [&] () {});
}

void
LocalizationCHT () {
    TestMode::RegisterModify (
        L"/root/menu[@id='TopMenu']/layout[@type='Center']/menu-item[@menu='ModManagerMenu']",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"模組管理"); }, [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Header']/text-item",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"模組管理"); }, [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModFreezeTimer']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"凍結計時");
            node.attribute (L"replace-text").set_value (L"0:關閉, 1:開啓");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo024']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"鬼滅之刃模式");
            node.attribute (L"replace-text").set_value (L"0:黙認, 1:啓用, 2:僅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo025']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"航海王模式");
            node.attribute (L"replace-text").set_value (L"0:黙認, 1:啓用, 2:僅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo026']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"ＡＩ粗品模式");
            node.attribute (L"replace-text").set_value (L"0:黙認, 1:啓用, 2:僅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeAprilFool001']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"青春之達人模式");
            node.attribute (L"replace-text").set_value (L"0:黙認, 1:啓用, 2:僅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/menu-item[@menu='TopMenu']",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"離開"); }, [] () {});
}

void
LocalizationCHS () {
    TestMode::RegisterModify (
        L"/root/menu[@id='TopMenu']/layout[@type='Center']/menu-item[@menu='ModManagerMenu']",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"模组管理"); }, [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Header']/text-item",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"模组管理"); }, [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModFreezeTimer']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"冻结计时");
            node.attribute (L"replace-text").set_value (L"0:禁用, 1:启用");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo024']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"鬼灭之刃模式");
            node.attribute (L"replace-text").set_value (L"0:默认, 1:启用, 2:仅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo025']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"航海王模式");
            node.attribute (L"replace-text").set_value (L"0:默认, 1:启用, 2:仅刷卡");
        },
        [] () {});
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeCollabo026']",
        [&] (const pugi::xml_node &node) {
            node.attribute (L"label").set_value (L"ＡＩ粗品模式");
            node.attribute (L"replace-text").set_value (L"0:默认, 1:启用, 2:仅刷卡");
        },
        [] () {});
    // TestMode::RegisterModify(
    //     L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/select-item[@id='ModModeAprilFool001']",
    //     [&](pugi::xml_node &node) { node.attribute(L"label").set_value(L"青春之达人模式"); node.attribute(L"replace-text").set_value(L"0:默认,
    //     1:启用, 2:仅刷卡"); }, [](){}
    // );
    TestMode::RegisterModify (
        L"/root/menu[@id='ModManagerMenu']/layout[@type='Center']/menu-item[@menu='TopMenu']",
        [&] (const pugi::xml_node &node) { node.attribute (L"label").set_value (L"离开"); }, [] () {});
}

void
Init () {
    LogMessage (LogLevel::INFO, "Init TestMode patches");

    const auto configPath = std::filesystem::current_path () / "config.toml";
    const std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);

    const u64 testModeSetMenuAddress = PROC_ADDRESS_OFFSET ("TestModeLibrary.dll", 0x99D0);
    switch (gameVersion) {
    case GameVersion::UNKNOWN: break;
    case GameVersion::JPN00: break;
    case GameVersion::JPN08: break;
    case GameVersion::JPN39: {
        if (config_ptr) {
            if (const auto patches = openConfigSection (config_ptr.get (), "patches")) {
                if (const auto jpn39 = openConfigSection (patches, "jpn39")) chsPatch = readConfigBool (jpn39, "chs_patch", chsPatch);
            }
        }
        if (chsPatch) LocalizationCHT ();
    } break;
    case GameVersion::CHN00: break;
    }

    CommonModify ();

    INSTALL_HOOK_DYNAMIC (TestModeSetMenuHook, testModeSetMenuAddress);
}

void
SetupAccessor (const u64 appAccessor, const RefTestModeMain refTestMode) {
    patches::TestMode::appAccessor = appAccessor;
    patches::TestMode::refTestMode = refTestMode;
}

int
ReadTestModeValue (const wchar_t *itemId) {
    if (appAccessor) {
        if (const u64 testModeMain = refTestMode (appAccessor)) {
            int value   = 0;
            u64 *reader = *reinterpret_cast<u64 **> (testModeMain + 16);
            (*reinterpret_cast<void (__fastcall **) (u64 *, const wchar_t *, int *)> (*reader + 256)) (reader, itemId, &value);
            return value;
        }
    }
    LogMessage (LogLevel::ERROR, (std::wstring (L"Read TestMode(") + itemId + L") failed!").c_str ());
    return -1;
}

void
RegisterItem (const std::wstring &item, const std::function<void ()> &initMethod) {
    LogMessage (LogLevel::DEBUG, L"Register Item " + item);
    registeredItems.push_back (new RegisteredItem (item, initMethod));
}

void
RegisterModify (const std::wstring &query, const std::function<void (pugi::xml_node &)> &nodeModify, const std::function<void ()> &initMethod) {
    LogMessage (LogLevel::DEBUG, L"Register Modify " + query);
    registeredModifies.push_back (new RegisteredModify (query, nodeModify, initMethod));
}

void
Append (const pugi::xml_node &node, const wchar_t *attr, const std::wstring &append) {
    pugi::xml_attribute attribute = node.attribute (attr);
    const std::wstring attrValue        = std::wstring (attribute.value ()) + append;
    attribute.set_value (attrValue.c_str ());
}
}