#include "poll.h"

extern bool jpLayout;

struct KeyCodePair {
    const char *string;
    u8 keycode;
};
size_t ConfigKeyboardButtonsCount      = 0;
KeyCodePair *ConfigKeyboardButtons     = nullptr;
KeyCodePair ConfigKeyboardButtons_US[] = {
    // Reference:https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // Wayback Machine:https://web.archive.org/web/20231223135232/https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // Row 1
    {"ESCAPE", VK_ESCAPE},
    {"F1", VK_F1},
    {"F2", VK_F2},
    {"F3", VK_F3},
    {"F4", VK_F4},
    {"F5", VK_F5},
    {"F6", VK_F6},
    {"F7", VK_F7},
    {"F8", VK_F8},
    {"F9", VK_F9},
    {"F10", VK_F10},
    {"F11", VK_F11},
    {"F12", VK_F12},

    // Row 2
    {"`", VK_OEM_3},
    {"1", '1'},
    {"2", '2'},
    {"3", '3'},
    {"4", '4'},
    {"5", '5'},
    {"6", '6'},
    {"7", '7'},
    {"8", '8'},
    {"9", '9'},
    {"0", '0'},
    {"-", VK_OEM_MINUS},
    {"=", VK_OEM_PLUS},
    {"BACKSPACE", VK_BACK},

    // Row 3
    {"TAB", VK_TAB},
    {"Q", 'Q'},
    {"W", 'W'},
    {"E", 'E'},
    {"R", 'R'},
    {"T", 'T'},
    {"Y", 'Y'},
    {"U", 'U'},
    {"I", 'I'},
    {"O", 'O'},
    {"P", 'P'},
    {"[", VK_OEM_4},
    {"]", VK_OEM_6},
    {"BACKSLASH", VK_OEM_5},

    // Row 4
    {"CAPS_LOCK", VK_CAPITAL},
    {"A", 'A'},
    {"S", 'S'},
    {"D", 'D'},
    {"F", 'F'},
    {"G", 'G'},
    {"H", 'H'},
    {"J", 'J'},
    {"K", 'K'},
    {"L", 'L'},
    {";", VK_OEM_1},
    {"'", VK_OEM_7},
    {"ENTER", VK_RETURN},

    // Row 5
    {"SHIFT", VK_SHIFT},
    {"Z", 'Z'},
    {"X", 'X'},
    {"C", 'C'},
    {"V", 'V'},
    {"B", 'B'},
    {"N", 'N'},
    {"M", 'M'},
    {",", VK_OEM_COMMA},
    {".", VK_OEM_PERIOD},
    {"SLASH", VK_OEM_2},

    // Row 6
    {"CONTROL", VK_CONTROL},
    {"L_WIN", VK_LWIN},
    {"ALT", VK_MENU},
    {"SPACE", VK_SPACE},
    {"R_WIN", VK_RWIN},
    {"MENU", VK_APPS},

    // Other Keys
    // PrtSc is more important when making snapshots, therefore comment it as reserved
    //{"PRINT_SCREEN", VK_SNAPSHOT},
    {"SCROLL_LOCK", VK_SCROLL},
    {"PAUSE", VK_PAUSE},
    {"INSERT", VK_INSERT},
    {"DELETE", VK_DELETE},
    {"HOME", VK_HOME},
    {"END", VK_END},
    {"PAGE_UP", VK_PRIOR},
    {"PAGE_DOWN", VK_NEXT},

    // Arrow Keys
    {"UPARROW", VK_UP},
    {"LEFTARROW", VK_LEFT},
    {"DOWNARROW", VK_DOWN},
    {"RIGHTARROW", VK_RIGHT},

    // NUMPAD Keys
    {"NUM_LOCK", VK_NUMLOCK},
    {"DIVIDE", VK_DIVIDE},
    {"MULTIPLY", VK_MULTIPLY},
    {"SUBTRACT", VK_SUBTRACT},
    {"NUM7", VK_NUMPAD7},
    {"NUM8", VK_NUMPAD8},
    {"NUM9", VK_NUMPAD9},
    {"ADD", VK_ADD},
    {"NUM4", VK_NUMPAD4},
    {"NUM5", VK_NUMPAD5},
    {"NUM6", VK_NUMPAD6},
    {"NUM1", VK_NUMPAD1},
    {"NUM2", VK_NUMPAD2},
    {"NUM3", VK_NUMPAD3},
    {"NUM0", VK_NUMPAD0},
    {"DECIMAL", VK_DECIMAL},
};
KeyCodePair ConfigKeyboardButtons_JP[] = {
    // Reference:https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // Wayback Machine:https://web.archive.org/web/20231223135232/https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // Row 1
    {"ESCAPE", VK_ESCAPE},
    {"F1", VK_F1},
    {"F2", VK_F2},
    {"F3", VK_F3},
    {"F4", VK_F4},
    {"F5", VK_F5},
    {"F6", VK_F6},
    {"F7", VK_F7},
    {"F8", VK_F8},
    {"F9", VK_F9},
    {"F10", VK_F10},
    {"F11", VK_F11},
    {"F12", VK_F12},

    // Row 2
    {"1", '1'},
    {"2", '2'},
    {"3", '3'},
    {"4", '4'},
    {"5", '5'},
    {"6", '6'},
    {"7", '7'},
    {"8", '8'},
    {"9", '9'},
    {"0", '0'},
    {"-", VK_OEM_MINUS},
    {"^", VK_OEM_7},
    {"YEN", VK_OEM_5},
    {"BACKSPACE", VK_BACK},

    // Row 3
    {"TAB", VK_TAB},
    {"Q", 'Q'},
    {"W", 'W'},
    {"E", 'E'},
    {"R", 'R'},
    {"T", 'T'},
    {"Y", 'Y'},
    {"U", 'U'},
    {"I", 'I'},
    {"O", 'O'},
    {"P", 'P'},
    {"@", VK_OEM_3},
    {"[", VK_OEM_4},

    // Row 4
    {"CAPS_LOCK", VK_CAPITAL},
    {"A", 'A'},
    {"S", 'S'},
    {"D", 'D'},
    {"F", 'F'},
    {"G", 'G'},
    {"H", 'H'},
    {"J", 'J'},
    {"K", 'K'},
    {"L", 'L'},
    {";", VK_OEM_PLUS},
    {":", VK_OEM_1},
    {"]", VK_OEM_6},
    {"ENTER", VK_RETURN},

    // Row 5
    {"SHIFT", VK_SHIFT},
    {"Z", 'Z'},
    {"X", 'X'},
    {"C", 'C'},
    {"V", 'V'},
    {"B", 'B'},
    {"N", 'N'},
    {"M", 'M'},
    {",", VK_OEM_COMMA},
    {".", VK_OEM_PERIOD},
    {"SLASH", VK_OEM_2},
    {"BACKSLASH", VK_OEM_102},

    // Row 6
    {"CONTROL", VK_CONTROL},
    {"L_WIN", VK_LWIN},
    {"ALT", VK_MENU},
    {"SPACE", VK_SPACE},
    {"R_WIN", VK_RWIN},
    {"MENU", VK_APPS},

    // Other Keys
    // PrtSc is more important when making snapshots, therefore comment it as reserved
    //{"PRINT_SCREEN", VK_SNAPSHOT},
    {"SCROLL_LOCK", VK_SCROLL},
    {"PAUSE", VK_PAUSE},
    {"INSERT", VK_INSERT},
    {"DELETE", VK_DELETE},
    {"HOME", VK_HOME},
    {"END", VK_END},
    {"PAGE_UP", VK_PRIOR},
    {"PAGE_DOWN", VK_NEXT},

    // Arrow Keys
    {"UPARROW", VK_UP},
    {"LEFTARROW", VK_LEFT},
    {"DOWNARROW", VK_DOWN},
    {"RIGHTARROW", VK_RIGHT},

    // NUMPAD Keys
    {"NUM_LOCK", VK_NUMLOCK},
    {"DIVIDE", VK_DIVIDE},
    {"MULTIPLY", VK_MULTIPLY},
    {"SUBTRACT", VK_SUBTRACT},
    {"NUM7", VK_NUMPAD7},
    {"NUM8", VK_NUMPAD8},
    {"NUM9", VK_NUMPAD9},
    {"ADD", VK_ADD},
    {"NUM4", VK_NUMPAD4},
    {"NUM5", VK_NUMPAD5},
    {"NUM6", VK_NUMPAD6},
    {"NUM1", VK_NUMPAD1},
    {"NUM2", VK_NUMPAD2},
    {"NUM3", VK_NUMPAD3},
    {"NUM0", VK_NUMPAD0},
    {"DECIMAL", VK_DECIMAL},
};

struct {
    const char *string;
    SDL_GameControllerButton button;
} ConfigControllerButtons[] = {
    {"SDL_A", SDL_CONTROLLER_BUTTON_A},
    {"SDL_B", SDL_CONTROLLER_BUTTON_B},
    {"SDL_X", SDL_CONTROLLER_BUTTON_X},
    {"SDL_Y", SDL_CONTROLLER_BUTTON_Y},
    {"SDL_BACK", SDL_CONTROLLER_BUTTON_BACK},
    {"SDL_GUIDE", SDL_CONTROLLER_BUTTON_GUIDE},
    {"SDL_START", SDL_CONTROLLER_BUTTON_START},
    {"SDL_LSTICK_PRESS", SDL_CONTROLLER_BUTTON_LEFTSTICK},
    {"SDL_RSTICK_PRESS", SDL_CONTROLLER_BUTTON_RIGHTSTICK},
    {"SDL_LSHOULDER", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
    {"SDL_RSHOULDER", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
    {"SDL_DPAD_UP", SDL_CONTROLLER_BUTTON_DPAD_UP},
    {"SDL_DPAD_DOWN", SDL_CONTROLLER_BUTTON_DPAD_DOWN},
    {"SDL_DPAD_LEFT", SDL_CONTROLLER_BUTTON_DPAD_LEFT},
    {"SDL_DPAD_RIGHT", SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
    {"SDL_MISC", SDL_CONTROLLER_BUTTON_MISC1},
    {"SDL_PADDLE1", SDL_CONTROLLER_BUTTON_PADDLE1},
    {"SDL_PADDLE2", SDL_CONTROLLER_BUTTON_PADDLE2},
    {"SDL_PADDLE3", SDL_CONTROLLER_BUTTON_PADDLE3},
    {"SDL_PADDLE4", SDL_CONTROLLER_BUTTON_PADDLE4},
    {"SDL_TOUCHPAD", SDL_CONTROLLER_BUTTON_TOUCHPAD},
};

struct {
    const char *string;
    SDLAxis axis;
} ConfigControllerAXIS[] = {
    {"SDL_LSTICK_LEFT", SDL_AXIS_LEFT_LEFT},   {"SDL_LSTICK_UP", SDL_AXIS_LEFT_UP},        {"SDL_LSTICK_DOWN", SDL_AXIS_LEFT_DOWN},
    {"SDL_LSTICK_RIGHT", SDL_AXIS_LEFT_RIGHT}, {"SDL_RSTICK_LEFT", SDL_AXIS_RIGHT_LEFT},   {"SDL_RSTICK_UP", SDL_AXIS_RIGHT_UP},
    {"SDL_RSTICK_DOWN", SDL_AXIS_RIGHT_DOWN},  {"SDL_RSTICK_RIGHT", SDL_AXIS_RIGHT_RIGHT}, {"SDL_LTRIGGER", SDL_AXIS_LTRIGGER_DOWN},
    {"SDL_RTRIGGER", SDL_AXIS_RTRIGGER_DOWN},
};

struct {
    const char *string;
    Scroll scroll;
} ConfigMouseScroll[] = {
    {"SCROLL_UP", MOUSE_SCROLL_UP},
    {"SCROLL_DOWN", MOUSE_SCROLL_DOWN},
};

struct MouseState {
    POINT Position;
    POINT RelativePosition;
    bool ScrolledUp;
    bool ScrolledDown;
} currentMouseState, lastMouseState;

bool currentKeyboardState[0xFF];
bool lastKeyboardState[0xFF];

bool currentControllerButtonsState[SDL_CONTROLLER_BUTTON_MAX];
bool lastControllerButtonsState[SDL_CONTROLLER_BUTTON_MAX];
SDLAxisState currentControllerAxisState;
SDLAxisState lastControllerAxisState;

SDL_Window *window;
SDL_GameController *controllers[255];

void
SetKeyboardButtons () {
    ConfigKeyboardButtonsCount = jpLayout ? std::size (ConfigKeyboardButtons_JP) : std::size (ConfigKeyboardButtons_US);
    ConfigKeyboardButtons      = static_cast<KeyCodePair *> (malloc (ConfigKeyboardButtonsCount * sizeof (KeyCodePair)));
    memcpy (ConfigKeyboardButtons, jpLayout ? ConfigKeyboardButtons_JP : ConfigKeyboardButtons_US, ConfigKeyboardButtonsCount * sizeof (KeyCodePair));
}

void
SetConfigValue (const toml_table_t *table, const char *key, Keybindings *key_bind) {
    const toml_array_t *array = toml_array_in (table, key);
    if (!array) {
        LogMessage (LogLevel::WARN, std::string (key) + ": Cannot find array");
        return;
    }

    memset (key_bind, 0, sizeof (*key_bind));
    for (size_t i = 0; i < std::size (key_bind->buttons); i++)
        key_bind->buttons[i] = SDL_CONTROLLER_BUTTON_INVALID;

    for (int idx = 0;; idx++) {
        const auto [ok, u] = toml_string_at (array, idx);
        if (!ok) break;
        const ConfigValue value = StringToConfigEnum (u.s);
        free (u.s);

        switch (value.type) {
        case keycode: {
            for (int i = 0; i < std::size (key_bind->keycodes); i++) {
                if (key_bind->keycodes[i] == 0) {
                    key_bind->keycodes[i] = value.keycode;
                    break;
                }
            }
            break;
        }
        case button: {
            for (int i = 0; i < std::size (key_bind->buttons); i++) {
                if (key_bind->buttons[i] == SDL_CONTROLLER_BUTTON_INVALID) {
                    key_bind->buttons[i] = value.button;
                    break;
                }
            }
            break;
        }
        case axis: {
            for (int i = 0; i < std::size (key_bind->axis); i++) {
                if (key_bind->axis[i] == 0) {
                    key_bind->axis[i] = value.axis;
                    break;
                }
            }
        }
        case scroll: {
            for (int i = 0; i < std::size (key_bind->scroll); i++) {
                if (key_bind->scroll[i] == 0) {
                    key_bind->scroll[i] = value.scroll;
                    break;
                }
            }
            break;
        }
        default: break;
        }
    }
}

bool
InitializePoll (const HWND windowHandle) {
    bool hasRumble = true;
    SDL_SetMainReady ();

    SDL_SetHint (SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint (SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    SDL_SetHint (SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint (SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");

    if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
        if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_VIDEO) == 0) {
            hasRumble = false;
        } else {
            LogMessage (LogLevel::ERROR,
                        std::string ("SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_VIDEO): ")
                            + SDL_GetError ());
            return false;
        }
    }

    if (const auto configPath = std::filesystem::current_path () / "gamecontrollerdb.txt";
        SDL_GameControllerAddMappingsFromFile (configPath.string ().c_str ()) == -1)
        LogMessage (LogLevel::ERROR, "Cannot read gamecontrollerdb.txt");
    SDL_GameControllerEventState (SDL_ENABLE);
    SDL_JoystickEventState (SDL_ENABLE);

    for (int i = 0; i < SDL_NumJoysticks (); i++) {
        if (!SDL_IsGameController (i)) continue;

        SDL_GameController *controller = SDL_GameControllerOpen (i);
        if (!controller) {
            LogMessage (LogLevel::WARN, std::string ("Could not open gamecontroller ") + SDL_GameControllerNameForIndex (i) + ": " + SDL_GetError ());
            continue;
        }
        controllers[i] = controller;
    }

    window = SDL_CreateWindowFrom (windowHandle);
    if (window == nullptr) LogMessage (LogLevel::ERROR, std::string ("SDL_CreateWindowFrom (windowHandle): ") + SDL_GetError ());
    atexit (DisposePoll);

    return hasRumble;
}

void
UpdatePoll (const HWND windowHandle) {
    if (windowHandle == nullptr || GetForegroundWindow () != windowHandle) return;

    memcpy (lastKeyboardState, currentKeyboardState, 255);
    memcpy (lastControllerButtonsState, currentControllerButtonsState, 21);
    lastMouseState          = currentMouseState;
    lastControllerAxisState = currentControllerAxisState;

    for (u8 i = 0; i < 0xFF; i++)
        currentKeyboardState[i] = GetAsyncKeyState (i) != 0;

    currentMouseState.ScrolledUp   = false;
    currentMouseState.ScrolledDown = false;

    GetCursorPos (&currentMouseState.Position);
    ScreenToClient (windowHandle, &currentMouseState.Position);

    SDL_Event event;
    SDL_GameController *controller;
    while (SDL_PollEvent (&event) != 0) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            if (!SDL_IsGameController (event.cdevice.which)) break;

            controller = SDL_GameControllerOpen (event.cdevice.which);
            if (!controller) {
                LogMessage (LogLevel::ERROR, std::string ("Could not open gamecontroller ") + SDL_GameControllerNameForIndex (event.cdevice.which)
                                                 + ": " + SDL_GetError ());
                continue;
            }
            controllers[event.cdevice.which] = controller;
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (!SDL_IsGameController (event.cdevice.which)) break;
            SDL_GameControllerClose (controllers[event.cdevice.which]);
            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0) currentMouseState.ScrolledUp = true;
            else if (event.wheel.y < 0) currentMouseState.ScrolledDown = true;
            break;
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN: currentControllerButtonsState[event.cbutton.button] = event.cbutton.state; break;
        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.value > 1) {
                switch (event.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX: currentControllerAxisState.LeftRight = static_cast<float> (event.caxis.value) / 32767; break;
                case SDL_CONTROLLER_AXIS_LEFTY: currentControllerAxisState.LeftDown = static_cast<float> (event.caxis.value) / 32767; break;
                case SDL_CONTROLLER_AXIS_RIGHTX: currentControllerAxisState.RightRight = static_cast<float> (event.caxis.value) / 32767; break;
                case SDL_CONTROLLER_AXIS_RIGHTY: currentControllerAxisState.RightDown = static_cast<float> (event.caxis.value) / 32767; break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT: currentControllerAxisState.LTriggerDown = static_cast<float> (event.caxis.value) / 32767; break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    currentControllerAxisState.RTriggerDown = static_cast<float> (event.caxis.value) / 32767;
                    break;
                default: break;
                }
            } else if (event.caxis.value < -1) {
                switch (event.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX: currentControllerAxisState.LeftLeft = static_cast<float> (event.caxis.value) / -32768; break;
                case SDL_CONTROLLER_AXIS_LEFTY: currentControllerAxisState.LeftUp = static_cast<float> (event.caxis.value) / -32768; break;
                case SDL_CONTROLLER_AXIS_RIGHTX: currentControllerAxisState.RightLeft = static_cast<float> (event.caxis.value) / -32768; break;
                case SDL_CONTROLLER_AXIS_RIGHTY: currentControllerAxisState.RightUp = static_cast<float> (event.caxis.value) / -32768; break;
                default: break;
                }
            } else {
                switch (event.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    currentControllerAxisState.LeftRight = 0;
                    currentControllerAxisState.LeftLeft  = 0;
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    currentControllerAxisState.LeftDown = 0;
                    currentControllerAxisState.LeftUp   = 0;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    currentControllerAxisState.RightRight = 0;
                    currentControllerAxisState.RightLeft  = 0;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    currentControllerAxisState.RightDown = 0;
                    currentControllerAxisState.RightUp   = 0;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT: currentControllerAxisState.LTriggerDown = 0; break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: currentControllerAxisState.RTriggerDown = 0; break;
                default: break;
                }
            }
            break;
        default: break;
        }
    }
}

void
DisposePoll () {
    SDL_DestroyWindow (window);
    SDL_Quit ();
}

ConfigValue
StringToConfigEnum (const char *value) {
    ConfigValue rval{};
    for (size_t i = 0; i < ConfigKeyboardButtonsCount; ++i)
        if (!strcmp (value, ConfigKeyboardButtons[i].string)) {
            rval.type    = keycode;
            rval.keycode = ConfigKeyboardButtons[i].keycode;
            return rval;
        }
    for (const auto &[string, button_] : ConfigControllerButtons)
        if (!strcmp (value, string)) {
            rval.type   = button;
            rval.button = button_;
            return rval;
        }
    for (const auto &[string, axis_] : ConfigControllerAXIS)
        if (!strcmp (value, string)) {
            rval.type = axis;
            rval.axis = axis_;
            return rval;
        }
    for (auto &[string, scroll_] : ConfigMouseScroll)
        if (!strcmp (value, string)) {
            rval.type   = scroll;
            rval.scroll = scroll_;
            return rval;
        }

    LogMessage (LogLevel::ERROR, std::string (value) + ": Unknown value");
    return rval;
}

InternalButtonState
GetInternalButtonState (const Keybindings &bindings) {
    InternalButtonState buttons = {};

    for (size_t i = 0; i < ConfigKeyboardButtonsCount; i++) {
        if (bindings.keycodes[i] == 0) continue;
        if (KeyboardIsReleased (bindings.keycodes[i])) buttons.Released = true;
        if (KeyboardIsDown (bindings.keycodes[i])) buttons.Down = 1;
        if (KeyboardIsTapped (bindings.keycodes[i])) buttons.Tapped = true;
    }
    for (size_t i = 0; i < std::size (ConfigControllerButtons); i++) {
        if (bindings.buttons[i] == SDL_CONTROLLER_BUTTON_INVALID) continue;
        if (ControllerButtonIsReleased (bindings.buttons[i])) buttons.Released = true;
        if (ControllerButtonIsDown (bindings.buttons[i])) buttons.Down = 1;
        if (ControllerButtonIsTapped (bindings.buttons[i])) buttons.Tapped = true;
    }
    for (size_t i = 0; i < std::size (ConfigControllerAXIS); i++) {
        if (bindings.axis[i] == 0) continue;
        if (const float val = ControllerAxisIsReleased (bindings.axis[i]))
            buttons.Released = static_cast<bool> (val);                                                            // NOLINT(*-narrowing-conversions)
        if (const float val = ControllerAxisIsDown (bindings.axis[i])) buttons.Down = val;                         // NOLINT(*-narrowing-conversions)
        if (const float val = ControllerAxisIsTapped (bindings.axis[i])) buttons.Tapped = static_cast<bool> (val); // NOLINT(*-narrowing-conversions)
    }
    for (const auto i : bindings.scroll) {
        if (i == 0) continue;
        if (GetMouseScrollIsReleased (i)) buttons.Released = true;
        if (GetMouseScrollIsDown (i)) buttons.Down = 1;
        if (GetMouseScrollIsTapped (i)) buttons.Tapped = true;
    }

    return buttons;
}

void
SetRumble (const int left, const int right, const int length) {
    for (auto &controller : controllers) {
        if (!controller || !SDL_GameControllerHasRumble (controller)) continue;

        SDL_GameControllerRumble (controller, left, right, length);
    }
}

bool
KeyboardIsDown (const u8 keycode) {
    return currentKeyboardState[keycode];
}

bool
KeyboardIsUp (const u8 keycode) {
    return !KeyboardIsDown (keycode);
}

bool
KeyboardIsTapped (const u8 keycode) {
    return KeyboardIsDown (keycode) && KeyboardWasUp (keycode);
}

bool
KeyboardIsReleased (const u8 keycode) {
    return KeyboardIsUp (keycode) && KeyboardWasDown (keycode);
}

bool
KeyboardWasDown (const u8 keycode) {
    return lastKeyboardState[keycode];
}

bool
KeyboardWasUp (const u8 keycode) {
    return !KeyboardWasDown (keycode);
}

POINT
GetMousePosition () { return currentMouseState.Position; }

POINT
GetLastMousePosition () { return lastMouseState.Position; }

POINT
GetMouseRelativePosition () { return currentMouseState.RelativePosition; }

POINT
GetLastMouseRelativePosition () { return lastMouseState.RelativePosition; }

void
SetMousePosition (const POINT newPosition) {
    currentMouseState.Position = newPosition;
}

bool
GetMouseScrollUp () {
    return currentMouseState.ScrolledUp;
}

bool
GetMouseScrollDown () {
    return currentMouseState.ScrolledDown;
}

bool
GetWasMouseScrollUp () {
    return lastMouseState.ScrolledUp;
}

bool
GetWasMouseScrollDown () {
    return lastMouseState.ScrolledDown;
}

bool
GetMouseScrollIsReleased (const Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return !GetMouseScrollUp () && GetWasMouseScrollUp ();
    return !GetMouseScrollDown () && GetWasMouseScrollDown ();
}

bool
GetMouseScrollIsDown (const Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return GetMouseScrollUp ();
    return GetMouseScrollDown ();
}

bool
GetMouseScrollIsTapped (const Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return GetMouseScrollUp () && !GetWasMouseScrollUp ();
    return GetMouseScrollDown () && !GetWasMouseScrollDown ();
}

bool
ControllerButtonIsDown (const SDL_GameControllerButton button) {
    return currentControllerButtonsState[button];
}

bool
ControllerButtonIsUp (const SDL_GameControllerButton button) {
    return !ControllerButtonIsDown (button);
}

bool
ControllerButtonWasDown (const SDL_GameControllerButton button) {
    return lastControllerButtonsState[button];
}

bool
ControllerButtonWasUp (const SDL_GameControllerButton button) {
    return !ControllerButtonWasDown (button);
}

bool
ControllerButtonIsTapped (const SDL_GameControllerButton button) {
    return ControllerButtonIsDown (button) && ControllerButtonWasUp (button);
}

bool
ControllerButtonIsReleased (const SDL_GameControllerButton button) {
    return ControllerButtonIsUp (button) && ControllerButtonWasDown (button);
}

float
ControllerAxisIsDown (const SDLAxis axis) {
    switch (axis) {
    case SDL_AXIS_LEFT_LEFT: return currentControllerAxisState.LeftLeft;
    case SDL_AXIS_LEFT_RIGHT: return currentControllerAxisState.LeftRight;
    case SDL_AXIS_LEFT_UP: return currentControllerAxisState.LeftUp;
    case SDL_AXIS_LEFT_DOWN: return currentControllerAxisState.LeftDown;
    case SDL_AXIS_RIGHT_LEFT: return currentControllerAxisState.RightLeft;
    case SDL_AXIS_RIGHT_RIGHT: return currentControllerAxisState.RightRight;
    case SDL_AXIS_RIGHT_UP: return currentControllerAxisState.RightUp;
    case SDL_AXIS_RIGHT_DOWN: return currentControllerAxisState.RightDown;
    case SDL_AXIS_LTRIGGER_DOWN: return currentControllerAxisState.LTriggerDown;
    case SDL_AXIS_RTRIGGER_DOWN: return currentControllerAxisState.RTriggerDown;
    default: return false;
    }
}

bool
ControllerAxisIsUp (const SDLAxis axis) {
    return !static_cast<bool> (ControllerAxisIsDown (axis));
}

float
ControllerAxisWasDown (const SDLAxis axis) {
    switch (axis) {
    case SDL_AXIS_LEFT_LEFT: return lastControllerAxisState.LeftLeft;
    case SDL_AXIS_LEFT_RIGHT: return lastControllerAxisState.LeftRight;
    case SDL_AXIS_LEFT_UP: return lastControllerAxisState.LeftUp;
    case SDL_AXIS_LEFT_DOWN: return lastControllerAxisState.LeftDown;
    case SDL_AXIS_RIGHT_LEFT: return lastControllerAxisState.RightLeft;
    case SDL_AXIS_RIGHT_RIGHT: return lastControllerAxisState.RightRight;
    case SDL_AXIS_RIGHT_UP: return lastControllerAxisState.RightUp;
    case SDL_AXIS_RIGHT_DOWN: return lastControllerAxisState.RightDown;
    case SDL_AXIS_LTRIGGER_DOWN: return lastControllerAxisState.LTriggerDown;
    case SDL_AXIS_RTRIGGER_DOWN: return lastControllerAxisState.RTriggerDown;
    default: return false;
    }
}

bool
ControllerAxisWasUp (const SDLAxis axis) {
    return !static_cast<bool> (ControllerAxisWasDown (axis));
}

bool
ControllerAxisIsTapped (const SDLAxis axis) {
    return static_cast<bool> (ControllerAxisIsDown (axis)) && ControllerAxisWasUp (axis);
}

bool
ControllerAxisIsReleased (const SDLAxis axis) {
    return ControllerAxisIsUp (axis) && static_cast<bool> (ControllerAxisWasDown (axis));
}

bool
IsButtonTapped (const Keybindings &bindings) {
    return GetInternalButtonState (bindings).Tapped;
}

bool
IsButtonReleased (const Keybindings &bindings) {
    return GetInternalButtonState (bindings).Released;
}

float
IsButtonDown (const Keybindings &bindings) {
    return GetInternalButtonState (bindings).Down;
}
