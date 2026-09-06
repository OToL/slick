#include "input.hpp"
#include "events.hpp"
#include <slk/math/utils.hpp>

#include <cassert>
#include <cstdlib>
#include <cstring>

import std;

namespace {
struct MouseState {
    static constexpr slk::u8 HAS_POSITION = 1 << 0;
    static constexpr slk::u8 HAS_PREV_POSITION = 1 << 1;

    slk::Vector2f m_prev_position;
    slk::Vector2f m_curr_postion;
    slk::MouseButtonMask m_prev_buttons_state;
    slk::MouseButtonMask m_curr_buttons_state;
    slk::Vector2f m_scroll;
    slk::u8 m_data_state;
};

struct GestureState {
    slk::u8 m_point_count;
    slk::Vector2f m_scroll_delta;
};

struct KeyboardState {
    static constexpr slk::u32 VKEY_COUNT = std::to_underlying(slk::KeyboardVKey::_COUNT);
    static constexpr slk::u32 VKEY_SIZE_BYTE = (VKEY_COUNT + 7U) >> 3U;

    static slk::u32 getVKeyByteIndex(slk::KeyboardVKey key) {
        return std::to_underlying(key) >> 3U;
    }

    static slk::u8 getVKeyByteOffset(slk::KeyboardVKey key) {
        return std::to_underlying(key) % 8;
    }

    slk::u8 curr_key_states[VKEY_SIZE_BYTE];
};

// TODO: change kb/mouse_events container type
// TODO: consider using a single container for all type of events e.g. raw queue
// TODO: should the queue of events be thread safe
struct InputAPIState {
    MouseState m_mouse_state;
    KeyboardState m_keyboard_state;
    GestureState m_gesture_state;
    std::vector<slk::GestureEvent> m_gesture_events;
    std::vector<slk::MouseEvent> m_mouse_events;
    std::vector<slk::KeyboardEvent> m_keyboard_events;
    slk::InputModifierMask m_modififers;
}* g_state = nullptr;
} // namespace

namespace slk {

b8 InputAPI::initialize() {
    assert(!g_state);

    g_state = new InputAPIState{};
    g_state->m_mouse_events.reserve(128);
    g_state->m_keyboard_events.reserve(128);
    g_state->m_gesture_events.reserve(5);

    return true;
}

b8 InputAPI::shutdown() {
    assert(g_state);

    delete (g_state);
    g_state = nullptr;

    return false;
}

static constexpr u8 MB_MODIFIER_OFFSET = 4U;
static constexpr u8 KB_MODIFIERS_COUNT = MB_MODIFIER_OFFSET;

static_assert((MB_MODIFIER_OFFSET + std::to_underlying(MouseButton::LEFT)) == std::to_underlying(InputModifier::LMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(MouseButton::MIDDLE)) == std::to_underlying(InputModifier::MMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(MouseButton::RIGHT)) == std::to_underlying(InputModifier::RMB));

void processEvents() {
    GestureState& gesture_state = g_state->m_gesture_state;

    u8 max_point_count = 0;
    for (GestureEvent const& evt : g_state->m_gesture_events) {
        // end of all movements
        if (evt.m_point_count == 0)
        {
            gesture_state.m_point_count = 0;
            break;
        }
        else {
            // priority to the gesture with most touches
            max_point_count = slk::max(evt.m_point_count, max_point_count);
            gesture_state.m_point_count = max_point_count;

            // apply the gesture effect only if this is the current one
            if (gesture_state.m_point_count == max_point_count) {
                if (evt.m_type == InputEventType::GESTURE_SCROLL)
                    gesture_state.m_scroll_delta = evt.m_delta;
            }
        }
    }

    MouseState& mouse_state = g_state->m_mouse_state;
    for (MouseEvent const& evt : g_state->m_mouse_events) {
        if (evt.m_type == InputEventType::MOUSE_MOVE) {
            mouse_state.m_curr_postion = evt.m_postion;
            mouse_state.m_data_state |= MouseState::HAS_POSITION;
        } else if (evt.m_type == InputEventType::MOUSE_BUTTON_DOWN) {
            mouse_state.m_curr_buttons_state |= static_cast<MouseButtonMask>(1U << std::to_underlying(evt.m_button));
            g_state->m_modififers |= static_cast<InputModifierMask>(1U << (std::to_underlying(evt.m_button) + MB_MODIFIER_OFFSET));
        } else if (evt.m_type == InputEventType::MOUSE_BUTTON_UP) {
            mouse_state.m_curr_buttons_state &= ~static_cast<MouseButtonMask>(1U << std::to_underlying(evt.m_button));
            g_state->m_modififers &= ~static_cast<InputModifierMask>(1U << (std::to_underlying(evt.m_button) + MB_MODIFIER_OFFSET));
        } else if (evt.m_type == InputEventType::MOUSE_SCROLL) {
            mouse_state.m_scroll += evt.m_scroll;
        } else {
            assert(false);
        }
    }

    u8 keyboard_modifiers[KB_MODIFIERS_COUNT] = {};
    KeyboardState& keyboard_state = g_state->m_keyboard_state;
    for (KeyboardEvent const& evt : g_state->m_keyboard_events) {
        i8 modifier_val = 1;
        if (evt.m_type == InputEventType::KEYBOARD_KEY_DOWN) {
            u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.m_vkey);
            u8 const byte_offset = KeyboardState::getVKeyByteOffset(evt.m_vkey);

            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);
            keyboard_state.curr_key_states[byte_idx] |= 1U << byte_offset;
        } else if (evt.m_type == InputEventType::KEYBOARD_KEY_UP) {
            u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.m_vkey);
            u8 const byte_offset = KeyboardState::getVKeyByteOffset(evt.m_vkey);
            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

            keyboard_state.curr_key_states[byte_idx] &= ~(1U << byte_offset);
            modifier_val = -1;
        } else {
            assert(false);
        }

        if (evt.m_vkey == KeyboardVKey::LEFT_CONTROL || evt.m_vkey == KeyboardVKey::RIGHT_CONTROL) {
            keyboard_modifiers[std::to_underlying(InputModifier::CTRL)] += modifier_val;
        }
        if (evt.m_vkey == KeyboardVKey::LEFT_SHIFT || evt.m_vkey == KeyboardVKey::RIGHT_SHIFT) {
            keyboard_modifiers[std::to_underlying(InputModifier::SHIFT)] += modifier_val;
        }
        if (evt.m_vkey == KeyboardVKey::LEFT_SUPER || evt.m_vkey == KeyboardVKey::RIGHT_SUPER) {
            keyboard_modifiers[std::to_underlying(InputModifier::SUPER)] += modifier_val;
        }
        if (evt.m_vkey == KeyboardVKey::LEFT_ALT || evt.m_vkey == KeyboardVKey::RIGHT_ALT) {
            keyboard_modifiers[std::to_underlying(InputModifier::ALT)] += modifier_val;
        }
    }

    for (u8 idx = 0; idx != KB_MODIFIERS_COUNT; ++idx) {
        i8 const mod = keyboard_modifiers[idx];
        if (mod < 0) {
            g_state->m_modififers &= ~static_cast<InputModifierMask>(1 << idx);
        } else if (mod > 0) {
            g_state->m_modififers |= static_cast<InputModifierMask>(1 << idx);
        }
    }

    g_state->m_keyboard_events.clear();
    g_state->m_mouse_events.clear();
    g_state->m_gesture_events.clear();
}

void InputAPI::update() {
    assert(g_state);

    MouseState& mouse_state = g_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_POSITION) {
        mouse_state.m_data_state |= MouseState::HAS_PREV_POSITION;
        mouse_state.m_prev_position = mouse_state.m_curr_postion;
    }
    mouse_state.m_prev_buttons_state = mouse_state.m_curr_buttons_state;
    mouse_state.m_scroll = Vector2f::ZERO;

    g_state->m_gesture_state.m_scroll_delta = Vector2f::ZERO;

    processEvents();
}

MouseButtonMask InputAPI::mouseButtonsState() {
    assert(g_state);
    return g_state->m_mouse_state.m_curr_buttons_state;
}

b8 InputAPI::areMouseButtonsDown(MouseButtonMask buttons) {
    assert(g_state);
    return (g_state->m_mouse_state.m_curr_buttons_state & buttons) == buttons;
}

b8 InputAPI::isMouseButtonDown(MouseButton button) {
    assert(g_state);
    return (g_state->m_mouse_state.m_curr_buttons_state & static_cast<MouseButtonMask>(1 << std::to_underlying(button))) != MouseButtonMask::NONE;
}

Vector2f InputAPI::mousePosition() {
    assert(g_state);
    return g_state->m_mouse_state.m_curr_postion;
}

Vector2f InputAPI::mouseMovement() {
    assert(g_state);

    MouseState& mouse_state = g_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_PREV_POSITION)
        return mouse_state.m_curr_postion - mouse_state.m_prev_position;

    return {};
}

Vector2f InputAPI::mouseScroll() {
    assert(g_state);
    return g_state->m_mouse_state.m_scroll;
}

Vector2f InputAPI::gestureScrollState() {
    assert(g_state);
    return g_state->m_gesture_state.m_scroll_delta;
}

u8 InputAPI::gestureTouchCount() {
    assert(g_state);
    return g_state->m_gesture_state.m_point_count;
}

b8 InputAPI::areKeyboardKeysDown(std::span<KeyboardVKey const> key_codes)
{
    for (auto key_code : key_codes)
    {
        if (!isKeyboardKeyDown(key_code))
            return false;
    }

    return true;
}

b8 InputAPI::isAnyKeyboardKeyDown(std::span<KeyboardVKey const> key_codes)
{
    for (auto key_code : key_codes)
    {
        if (isKeyboardKeyDown(key_code))
            return true;
    }

    return false;
}

b8 InputAPI::isKeyboardKeyDown(KeyboardVKey key_code) {
    assert(g_state);

    u32 const byte_idx = KeyboardState::getVKeyByteIndex(key_code);
    u8 const byte_offset = KeyboardState::getVKeyByteOffset(key_code);
    assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

    return 0 != (g_state->m_keyboard_state.curr_key_states[byte_idx] & (1U << byte_offset));
}

void InputAPI::forwardEvent(InputEvent const& evt) {
    assert(g_state);

    if ((evt.m_type >= InputEventType::MOUSE_BUTTON_UP) && (evt.m_type <= InputEventType::MOUSE_SCROLL)) {
        MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
        g_state->m_mouse_events.emplace_back(spec_evt);
    } else if ((evt.m_type >= InputEventType::KEYBOARD_KEY_UP) && (evt.m_type <= InputEventType::KEYBOARD_KEY_DOWN)) {
        KeyboardEvent const& spec_evt = static_cast<KeyboardEvent const&>(evt);
        g_state->m_keyboard_events.emplace_back(spec_evt);
    } 
    else if ((evt.m_type >= InputEventType::GESTURE_TOUCH_COUNT_UPDATE) && (evt.m_type <= InputEventType::GESTURE_SCROLL)) {
        GestureEvent const& spec_evt = static_cast<GestureEvent const&>(evt);
        g_state->m_gesture_events.emplace_back(spec_evt);
    }
    else {
        assert(false);
    }
}

InputModifierMask InputAPI::modifiersState() {
    assert(g_state);

    return g_state->m_modififers;
}

b8 InputAPI::hasAnyModifier(InputModifierMask mod_mask) {
    assert(g_state);

    return InputModifierMask::NONE != (g_state->m_modififers & mod_mask);
}

b8 InputAPI::hasModifier(InputModifier mod_key) {
    assert(g_state);

    return hasAnyModifier(static_cast<InputModifierMask>(1 << std::to_underlying(mod_key)));
}

}

