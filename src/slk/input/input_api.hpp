#pragma once

#include <slk/core.hpp>
#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

import std;

namespace slk {

struct InputEvent;

// TODO: Event based notification e.g. to properly handle key down repeat
class InputApi {

public:

    static b8 initialize();
    static b8 shutdown();
    static InputApi* instance() {return ms_instance;}

    void update();

    // Modifiers
    InputModifierMask modifiersState() const; 
    b8 hasAnyModifier(InputModifierMask mod_mask) const;
    b8 hasModifier(InputModifier mod_key) const;

    // Mouse
    Vector2f mousePosition() const;
    Vector2f mouseMovement() const;
    Vector2f mouseScroll() const;
    b8 isMouseButtonDown(MouseButton button) const;
    b8 areMouseButtonsDown(MouseButtonMask buttons) const;
    MouseButtonMask mouseButtonsState() const;

    // Gesture & Touchpad
    u8 gestureTouchCount() const;
    Vector2f gestureScrollState() const;

    // Keyboard
    // TODO: pressed & released
    b8 isKeyboardKeyDown(KeyboardVKey key_code) const;
    b8 isAnyKeyboardKeyDown(std::span<KeyboardVKey const> key_codes) const;
    b8 areKeyboardKeysDown(std::span<KeyboardVKey const> key_codes) const;

    // External input events injection
    void forwardEvent(InputEvent const& evt);

private:

    InputApi();
    ~InputApi();

    void processEvents();

    struct State;
    State* m_state = nullptr;

    static InputApi* ms_instance;
};

} // namespace slk
