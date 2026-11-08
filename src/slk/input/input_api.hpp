#pragma once

#include <slk/core.hpp>
#include <slk/api_registry.hpp>
#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

import std;

namespace slk {

struct InputEvent;

// TODO: pressed & released
// TODO: push events to client
class InputApi {

public:

    sb_declare_api(input);

    InputApi();
    ~InputApi();

    b8 initialize();
    b8 shutdown();
    void update();

    // Modifiers
    EInputModifierBits modifiersState() const; 
    b8 hasAnyModifier(EInputModifierBits mod_mask) const;
    b8 hasModifier(EInputModifier mod_key) const;

    // Mouse
    Vector2f mousePosition() const;
    Vector2f mouseMovement() const;
    Vector2f mouseScroll() const;
    EMouseButtonBits mouseButtonsState() const;
    b8 isMouseButtonDown(EMouseButton button) const;
    b8 areMouseButtonsDown(EMouseButtonBits buttons) const;

    // Keyboard
    b8 isKeyboardKeyDown(EKeyboardVKey key_code) const;
    b8 isAnyKeyboardKeyDown(std::span<EKeyboardVKey const> key_codes) const;
    b8 areKeyboardKeysDown(std::span<EKeyboardVKey const> key_codes) const;

    // Gesture & Touchpad
    u8 gestureTouchCount() const;
    Vector2f gestureScroll() const;
    Vector2f gestureMagnify() const;

    // External input events injection
    void forwardEvent(InputEvent const& evt);

    void processEvents();

    struct State;
    State* const m_state = nullptr;
};

inline InputApi& inputApi() {return api<InputApi>();}

} // namespace slk
