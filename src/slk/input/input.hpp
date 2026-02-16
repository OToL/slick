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
    static InputModifier::Mask getModifiersState(); 
    static b8 hasModifiers(InputModifier::Mask mod_mask);
    static b8 hasModifier(EInputModifier mod_key);

    // Mouse
    static Vector2f getMousePosition();
    static Vector2f getMouseMovement();
    static Vector2f getMouseScroll();
    static b8 isMouseButtonDown(EMouseButton button);
    static MouseButton::Mask getMouseButtonsState();

    // Keyboard
    // TODO: pressed & released
    static b8 isKeyboardKeyDown(EVirtualKey key_code);

    static void forwardEvent(InputEvent const& evt);
};

} // namespace slk
