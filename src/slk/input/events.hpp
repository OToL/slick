#pragma once

#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

namespace slk {

struct InputEvent {
    // Order of enum values is used to determine event type
    enum Type : u8 {
        MOUSE_BUTTON_UP,
        MOUSE_BUTTON_DOWN,
        MOUSE_MOVE,
        MOUSE_SCROLL,

        KEYBOARD_KEY_UP,
        KEYBOARD_KEY_DOWN,
        // KEYBOARD_KEY_CHAR, --> update forward evt

        // Keyboard
        // Touch
        // GamePad

        _LAST
    };

    Type type;
    // TODO: put common stuff there e.g. modifiers
};

struct MouseEvent : InputEvent {
    using InputEvent::type;

    union {
        // MOUSE_MOVE
        Vector2f postion;
        // MOUVE_SCROLL
        Vector2f scroll;
        // MOUSE_BUTTON_UP/DOWN
        EMouseButton button;
    };
};

struct KeyboardEvent : InputEvent {
    using InputEvent::type;

    EVirtualKey vkey;
    // Not currently used
    b8 key_repeat;
};


}
