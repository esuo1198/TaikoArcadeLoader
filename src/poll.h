#pragma once
#include <SDL.h>
#include "helpers.h"

enum SDLAxis {
    SDL_AXIS_NULL,
    SDL_AXIS_LEFT_LEFT,
    SDL_AXIS_LEFT_RIGHT,
    SDL_AXIS_LEFT_UP,
    SDL_AXIS_LEFT_DOWN,
    SDL_AXIS_RIGHT_LEFT,
    SDL_AXIS_RIGHT_RIGHT,
    SDL_AXIS_RIGHT_UP,
    SDL_AXIS_RIGHT_DOWN,
    SDL_AXIS_LTRIGGER_DOWN,
    SDL_AXIS_RTRIGGER_DOWN,
    SDL_AXIS_MAX
};

struct SDLAxisState {
    float LeftLeft;
    float LeftRight;
    float LeftUp;
    float LeftDown;
    float RightLeft;
    float RightRight;
    float RightUp;
    float RightDown;
    float LTriggerDown;
    float RTriggerDown;
};

enum Scroll { MOUSE_SCROLL_INVALID, MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN };

struct Keybindings {
    u8 keycodes[255];
    SDL_GameControllerButton buttons[255];
    SDLAxis axis[255];
    Scroll scroll[2];
};

enum EnumType { none, keycode, button, axis, scroll };

struct ConfigValue {
    EnumType type;
    union {
        u8 keycode;
        SDL_GameControllerButton button;
        SDLAxis axis;
        Scroll scroll;
    };
};

struct InternalButtonState {
    float Down;
    bool Released;
    bool Tapped;
};

bool InitializePoll (HWND windowHandle);
void UpdatePoll (HWND windowHandle);
void DisposePoll ();
void SetKeyboardButtons ();
ConfigValue StringToConfigEnum (const char *value);
void SetConfigValue (const toml_table_t *table, const char *key, Keybindings *key_bind);
InternalButtonState GetInternalButtonState (const Keybindings &bindings);
void SetRumble (int left, int right, int length);

bool KeyboardIsDown (u8 keycode);
bool KeyboardIsUp (u8 keycode);
bool KeyboardIsTapped (u8 keycode);
bool KeyboardIsReleased (u8 keycode);
bool KeyboardWasDown (u8 keycode);
bool KeyboardWasUp (u8 keycode);
POINT GetMousePosition ();
POINT GetLastMousePosition ();
POINT GetMouseRelativePosition ();
POINT GetLastMouseRelativePosition ();
void SetMousePosition (POINT newPosition);
bool GetMouseScrollUp ();
bool GetMouseScrollDown ();
bool GetWasMouseScrollUp ();
bool GetWasMouseScrollDown ();
bool GetMouseScrollIsReleased (Scroll scroll);
bool GetMouseScrollIsDown (Scroll scroll);
bool GetMouseScrollIsTapped (Scroll scroll);
bool ControllerButtonIsDown (SDL_GameControllerButton button);
bool ControllerButtonIsUp (SDL_GameControllerButton button);
bool ControllerButtonWasDown (SDL_GameControllerButton button);
bool ControllerButtonWasUp (SDL_GameControllerButton button);
bool ControllerButtonIsTapped (SDL_GameControllerButton button);
bool ControllerButtonIsReleased (SDL_GameControllerButton button);
float ControllerAxisIsDown (SDLAxis axis);
bool ControllerAxisIsUp (SDLAxis axis);
float ControllerAxisWasDown (SDLAxis axis);
bool ControllerAxisWasUp (SDLAxis axis);
bool ControllerAxisIsTapped (SDLAxis axis);
bool ControllerAxisIsReleased (SDLAxis axis);
bool IsButtonTapped (const Keybindings &bindings);
bool IsButtonReleased (const Keybindings &bindings);
float IsButtonDown (const Keybindings &bindings);
