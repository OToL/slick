#pragma once

#include <slk/core.hpp>
#include <slk/api.hpp>
#include <slk/math/vector2.hpp>
#include "codes.hpp"

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
    EInputModifierBits modifiersDown() const; 

    // Mouse
    Vector2f mousePosition() const;
    Vector2f mouseMovement() const;
    Vector2f mouseScroll() const;
    EMouseButtonBits mouseButtonsDown() const;

    // Keyboard
    b8 isKeyboardKeyDown(EKeyboardVKey key_code) const;
    b8 anyKeyboardKeyDown(std::span<EKeyboardVKey const> key_codes) const;
    b8 allKeyboardKeysDown(std::span<EKeyboardVKey const> key_codes) const;

    // Gesture & Touchpad
    u8 gestureTouchCount() const;
    Vector2f gestureScroll() const;
    Vector2f gestureMagnify() const;

    // External input events injection
    void forwardEvent(InputEvent const& evt);

    struct State;
    State* const m_state = nullptr;
};

inline InputApi& inputApi() {return api<InputApi>();}

} // namespace slk
