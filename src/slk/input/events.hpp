#pragma once

#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

namespace slk {

// Order of enum values is used to determine event type (i.e. mouse, gesture, keyboard, etc.)
enum class InputEventType : u8 {
    MOUSE_BUTTON_UP,
    MOUSE_BUTTON_DOWN,
    MOUSE_MOVE,
    MOUSE_SCROLL,

    // TODO: implement magnify
    GESTURE_TOUCH_COUNT_UPDATE,
    GESTURE_SCROLL,

    KEYBOARD_KEY_UP,
    KEYBOARD_KEY_DOWN,
    // TODO: KEYBOARD_KEY_CHAR, --> update forward evt

    // TODO: GamePad

    _COUNT
};

struct InputEvent {
    InputEventType m_type;
};

struct GestureEvent : InputEvent {
    using InputEvent::m_type;

    u8 m_point_count;
    Vector2f m_delta;
};

struct MouseEvent : InputEvent {
    using InputEvent::m_type;

    union {
        // MOUSE_MOVE
        Vector2f m_postion;
        // MOUVE_SCROLL
        Vector2f m_scroll;
        // MOUSE_BUTTON_UP/DOWN
        MouseButton m_button;
    };
};

struct KeyboardEvent : InputEvent {
    using InputEvent::m_type;

    KeyboardVKey m_vkey;
    // Not currently used
    b8 m_key_repeat;
};

} // namespace slk
