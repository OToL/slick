#pragma once

#include <slk/core.hpp>
#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

import std;

namespace slk {

struct InputEvent;

// TODO: Event based notification e.g. to properly handle key down repeat
struct InputAPI {
    InputAPI() = delete;

    static b8 initialize();
    static b8 shutdown();
    static void update();

    // Modifiers
    static InputModifierMask modifiersState(); 
    static b8 hasAnyModifier(InputModifierMask mod_mask);
    static b8 hasModifier(InputModifier mod_key);

    // Mouse
    static Vector2f mousePosition();
    static Vector2f mouseMovement();
    static Vector2f mouseScroll();
    static b8 isMouseButtonDown(MouseButton button);
    static b8 areMouseButtonsDown(MouseButtonMask buttons);
    static MouseButtonMask mouseButtonsState();

    // Gesture & Touchpad
    static u8 gestureTouchCount();
    static Vector2f gestureScrollState();

    // Keyboard
    // TODO: pressed & released
    static b8 isKeyboardKeyDown(KeyboardVKey key_code);
    static b8 isAnyKeyboardKeyDown(std::span<KeyboardVKey const> key_codes);
    static b8 areKeyboardKeysDown(std::span<KeyboardVKey const> key_codes);

    // External input events injection
    static void forwardEvent(InputEvent const& evt);
};

} // namespace slk
