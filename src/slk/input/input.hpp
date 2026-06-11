#pragma once

#include <slk/core.hpp>
#include <slk/math/vector2.hpp>
#include "key_codes.hpp"

namespace slk {

struct InputEvent;

// TODO: Event based notification e.g. to properly handle key down repeat
struct InputAPI {
    InputAPI() = delete;

    static b8 init();
    static b8 shutdown();
    static void update();

    // Modifiers
    static InputModifier::Mask get_modifiers_state(); 
    static b8 has_modifiers(InputModifier::Mask mod_mask);
    static b8 has_modifier(EInputModifier mod_key);

    // Mouse
    static Vector2f get_mouse_position();
    static Vector2f get_mouse_movement();
    static Vector2f get_mouse_scroll();
    static b8 is_mouse_button_down(EMouseButton button);
    static b8 are_mouse_buttons_down(MouseButton::Mask buttons);
    static MouseButton::Mask get_mouse_buttons_state();

    // Gesture & Touchpad
    static u8 get_touch_point_count();

    // Keyboard
    // TODO: pressed & released
    static b8 is_keyboard_key_down(EVirtualKey key_code);

    static void forward_event(InputEvent const& evt);
};

} // namespace slk
