#include "poll.h"
#include "helpers.h"
#include <windows.h>

extern bool jpLayout;

struct KeyCodePair {
    const char *string;
    uint8_t keycode;
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
    ConfigKeyboardButtonsCount = jpLayout ? COUNTOFARR (ConfigKeyboardButtons_JP) : COUNTOFARR (ConfigKeyboardButtons_US);
    ConfigKeyboardButtons      = (KeyCodePair *)malloc (ConfigKeyboardButtonsCount * sizeof (KeyCodePair));
    memcpy (ConfigKeyboardButtons, jpLayout ? ConfigKeyboardButtons_JP : ConfigKeyboardButtons_US, ConfigKeyboardButtonsCount * sizeof (KeyCodePair));
}

void
SetConfigValue (toml_table_t *table, const char *key, Keybindings *keybind) {
    toml_array_t *array = toml_array_in (table, key);
    if (!array) {
        LogMessage (LOG_LEVEL_WARN, (std::string (key) + ": Cannot find array").c_str ());
        return;
    }

    memset (keybind, 0, sizeof (*keybind));
    for (size_t i = 0; i < COUNTOFARR (keybind->buttons); i++)
        keybind->buttons[i] = SDL_CONTROLLER_BUTTON_INVALID;

    for (size_t i = 0;; i++) {
        toml_datum_t bind = toml_string_at (array, i);
        if (!bind.ok) break;
        ConfigValue value = StringToConfigEnum (bind.u.s);
        free (bind.u.s);

        switch (value.type) {
        case keycode:
            for (size_t i = 0; i < COUNTOFARR (keybind->keycodes); i++) {
                if (keybind->keycodes[i] == 0) {
                    keybind->keycodes[i] = value.keycode;
                    break;
                }
            }
            break;
        case button:
            for (size_t i = 0; i < COUNTOFARR (keybind->buttons); i++) {
                if (keybind->buttons[i] == SDL_CONTROLLER_BUTTON_INVALID) {
                    keybind->buttons[i] = value.button;
                    break;
                }
            }
            break;
        case axis:
            for (size_t i = 0; i < COUNTOFARR (keybind->axis); i++) {
                if (keybind->axis[i] == 0) {
                    keybind->axis[i] = value.axis;
                    break;
                }
            }
        case scroll:
            for (size_t i = 0; i < COUNTOFARR (keybind->scroll); i++) {
                if (keybind->scroll[i] == 0) {
                    keybind->scroll[i] = value.scroll;
                    break;
                }
            }
            break;
        default: break;
        }
    }
}

bool
InitializePoll (HWND windowHandle) {
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
            LogMessage (LOG_LEVEL_ERROR,
                        (std::string ("SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_VIDEO): ")
                         + SDL_GetError ())
                            .c_str ());
            return false;
        }
    }

    auto configPath = std::filesystem::current_path () / "gamecontrollerdb.txt";
    if (SDL_GameControllerAddMappingsFromFile (configPath.string ().c_str ()) == -1) LogMessage (LOG_LEVEL_ERROR, "Cannot read gamecontrollerdb.txt");
    SDL_GameControllerEventState (SDL_ENABLE);
    SDL_JoystickEventState (SDL_ENABLE);

    for (int i = 0; i < SDL_NumJoysticks (); i++) {
        if (!SDL_IsGameController (i)) continue;

        SDL_GameController *controller = SDL_GameControllerOpen (i);
        if (!controller) {
            LogMessage (LOG_LEVEL_WARN,
                        (std::string ("Could not open gamecontroller ") + SDL_GameControllerNameForIndex (i) + ": " + SDL_GetError ()).c_str ());
            continue;
        }
        controllers[i] = controller;
    }

    window = SDL_CreateWindowFrom (windowHandle);
    if (window == NULL) LogMessage (LOG_LEVEL_ERROR, (std::string ("SDL_CreateWindowFrom (windowHandle): ") + SDL_GetError ()).c_str ());
    atexit (DisposePoll);

    return hasRumble;
}

void
UpdatePoll (HWND windowHandle) {
    if (windowHandle == NULL || GetForegroundWindow () != windowHandle) return;

    memcpy (lastKeyboardState, currentKeyboardState, 255);
    memcpy (lastControllerButtonsState, currentControllerButtonsState, 21);
    lastMouseState          = currentMouseState;
    lastControllerAxisState = currentControllerAxisState;

    for (uint8_t i = 0; i < 0xFF; i++)
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
                LogMessage (LOG_LEVEL_ERROR, (std::string ("Could not open gamecontroller ") + SDL_GameControllerNameForIndex (event.cdevice.which)
                                              + ": " + SDL_GetError ())
                                                 .c_str ());
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
                case SDL_CONTROLLER_AXIS_LEFTX: currentControllerAxisState.LeftRight = (float)event.caxis.value / 32767; break;
                case SDL_CONTROLLER_AXIS_LEFTY: currentControllerAxisState.LeftDown = (float)event.caxis.value / 32767; break;
                case SDL_CONTROLLER_AXIS_RIGHTX: currentControllerAxisState.RightRight = (float)event.caxis.value / 32767; break;
                case SDL_CONTROLLER_AXIS_RIGHTY: currentControllerAxisState.RightDown = (float)event.caxis.value / 32767; break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT: currentControllerAxisState.LTriggerDown = (float)event.caxis.value / 32767; break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: currentControllerAxisState.RTriggerDown = (float)event.caxis.value / 32767; break;
                }
            } else if (event.caxis.value < -1) {
                switch (event.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX: currentControllerAxisState.LeftLeft = (float)event.caxis.value / -32768; break;
                case SDL_CONTROLLER_AXIS_LEFTY: currentControllerAxisState.LeftUp = (float)event.caxis.value / -32768; break;
                case SDL_CONTROLLER_AXIS_RIGHTX: currentControllerAxisState.RightLeft = (float)event.caxis.value / -32768; break;
                case SDL_CONTROLLER_AXIS_RIGHTY: currentControllerAxisState.RightUp = (float)event.caxis.value / -32768; break;
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
                }
            }
            break;
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
    ConfigValue rval;
    for (size_t i = 0; i < ConfigKeyboardButtonsCount; ++i)
        if (!strcmp (value, ConfigKeyboardButtons[i].string)) {
            rval.type    = keycode;
            rval.keycode = ConfigKeyboardButtons[i].keycode;
            return rval;
        }
    for (size_t i = 0; i < COUNTOFARR (ConfigControllerButtons); ++i)
        if (!strcmp (value, ConfigControllerButtons[i].string)) {
            rval.type   = button;
            rval.button = ConfigControllerButtons[i].button;
            return rval;
        }
    for (size_t i = 0; i < COUNTOFARR (ConfigControllerAXIS); ++i)
        if (!strcmp (value, ConfigControllerAXIS[i].string)) {
            rval.type = axis;
            rval.axis = ConfigControllerAXIS[i].axis;
            return rval;
        }
    for (size_t i = 0; i < COUNTOFARR (ConfigMouseScroll); ++i)
        if (!strcmp (value, ConfigMouseScroll[i].string)) {
            rval.type   = scroll;
            rval.scroll = ConfigMouseScroll[i].scroll;
            return rval;
        }

    LogMessage (LOG_LEVEL_ERROR, (std::string (value) + ": Unknown value").c_str ());
    return rval;
}

InternalButtonState
GetInternalButtonState (Keybindings bindings) {
    InternalButtonState buttons = {0};

    for (size_t i = 0; i < ConfigKeyboardButtonsCount; i++) {
        if (bindings.keycodes[i] == 0) continue;
        if (KeyboardIsReleased (bindings.keycodes[i])) buttons.Released = 1;
        if (KeyboardIsDown (bindings.keycodes[i])) buttons.Down = 1;
        if (KeyboardIsTapped (bindings.keycodes[i])) buttons.Tapped = 1;
    }
    for (size_t i = 0; i < COUNTOFARR (ConfigControllerButtons); i++) {
        if (bindings.buttons[i] == SDL_CONTROLLER_BUTTON_INVALID) continue;
        if (ControllerButtonIsReleased (bindings.buttons[i])) buttons.Released = 1;
        if (ControllerButtonIsDown (bindings.buttons[i])) buttons.Down = 1;
        if (ControllerButtonIsTapped (bindings.buttons[i])) buttons.Tapped = 1;
    }
    for (size_t i = 0; i < COUNTOFARR (ConfigControllerAXIS); i++) {
        if (bindings.axis[i] == 0) continue;
        if (float val = ControllerAxisIsReleased (bindings.axis[i])) buttons.Released = val;
        if (float val = ControllerAxisIsDown (bindings.axis[i])) buttons.Down = val;
        if (float val = ControllerAxisIsTapped (bindings.axis[i])) buttons.Tapped = val;
    }
    for (size_t i = 0; i < COUNTOFARR (ConfigMouseScroll); i++) {
        if (bindings.scroll[i] == 0) continue;
        if (GetMouseScrollIsReleased (bindings.scroll[i])) buttons.Released = 1;
        if (GetMouseScrollIsDown (bindings.scroll[i])) buttons.Down = 1;
        if (GetMouseScrollIsTapped (bindings.scroll[i])) buttons.Tapped = 1;
    }

    return buttons;
}

void
SetRumble (int left, int right, int length) {
    for (size_t i = 0; i < COUNTOFARR (controllers); i++) {
        if (!controllers[i] || !SDL_GameControllerHasRumble (controllers[i])) continue;

        SDL_GameControllerRumble (controllers[i], left, right, length);
    }
}

bool
KeyboardIsDown (uint8_t keycode) {
    return currentKeyboardState[keycode];
}

bool
KeyboardIsUp (uint8_t keycode) {
    return !KeyboardIsDown (keycode);
}

bool
KeyboardIsTapped (uint8_t keycode) {
    return KeyboardIsDown (keycode) && KeyboardWasUp (keycode);
}

bool
KeyboardIsReleased (uint8_t keycode) {
    return KeyboardIsUp (keycode) && KeyboardWasDown (keycode);
}

bool
KeyboardWasDown (uint8_t keycode) {
    return lastKeyboardState[keycode];
}

bool
KeyboardWasUp (uint8_t keycode) {
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
SetMousePosition (POINT newPosition) {
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
GetMouseScrollIsReleased (Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return !GetMouseScrollUp () && GetWasMouseScrollUp ();
    else return !GetMouseScrollDown () && GetWasMouseScrollDown ();
}

bool
GetMouseScrollIsDown (Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return GetMouseScrollUp ();
    else return GetMouseScrollDown ();
}

bool
GetMouseScrollIsTapped (Scroll scroll) {
    if (scroll == MOUSE_SCROLL_UP) return GetMouseScrollUp () && !GetWasMouseScrollUp ();
    else return GetMouseScrollDown () && !GetWasMouseScrollDown ();
}

bool
ControllerButtonIsDown (SDL_GameControllerButton button) {
    return currentControllerButtonsState[button];
}

bool
ControllerButtonIsUp (SDL_GameControllerButton button) {
    return !ControllerButtonIsDown (button);
}

bool
ControllerButtonWasDown (SDL_GameControllerButton button) {
    return lastControllerButtonsState[button];
}

bool
ControllerButtonWasUp (SDL_GameControllerButton button) {
    return !ControllerButtonWasDown (button);
}

bool
ControllerButtonIsTapped (SDL_GameControllerButton button) {
    return ControllerButtonIsDown (button) && ControllerButtonWasUp (button);
}

bool
ControllerButtonIsReleased (SDL_GameControllerButton button) {
    return ControllerButtonIsUp (button) && ControllerButtonWasDown (button);
}

float
ControllerAxisIsDown (SDLAxis axis) {
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
ControllerAxisIsUp (SDLAxis axis) {
    return !ControllerAxisIsDown (axis);
}

float
ControllerAxisWasDown (SDLAxis axis) {
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
ControllerAxisWasUp (SDLAxis axis) {
    return !ControllerAxisWasDown (axis);
}

bool
ControllerAxisIsTapped (SDLAxis axis) {
    return ControllerAxisIsDown (axis) && ControllerAxisWasUp (axis);
}

bool
ControllerAxisIsReleased (SDLAxis axis) {
    return ControllerAxisIsUp (axis) && ControllerAxisWasDown (axis);
}

bool
IsButtonTapped (Keybindings bindings) {
    return GetInternalButtonState (bindings).Tapped;
}

bool
IsButtonReleased (Keybindings bindings) {
    return GetInternalButtonState (bindings).Released;
}

float
IsButtonDown (Keybindings bindings) {
    return GetInternalButtonState (bindings).Down;
}
