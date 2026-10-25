#include "input_api.hpp"
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
    slk::EMouseButtonMask m_prev_buttons_state;
    slk::EMouseButtonMask m_curr_buttons_state;
    slk::Vector2f m_scroll;
    slk::u8 m_data_state;
};

struct GestureState {
    slk::u8 m_point_count;
    slk::Vector2f m_scroll_delta;
    slk::Vector2f m_magnify_delta;
};

struct KeyboardState {
    static constexpr slk::u32 VKEY_COUNT = std::to_underlying(slk::EKeyboardVKey::_COUNT);
    static constexpr slk::u32 VKEY_SIZE_BYTE = (VKEY_COUNT + 7U) >> 3U;

    static slk::u32 getVKeyByteIndex(slk::EKeyboardVKey key) {
        return std::to_underlying(key) >> 3U;
    }

    static slk::u8 getVKeyByteOffset(slk::EKeyboardVKey key) {
        return std::to_underlying(key) % 8;
    }

    slk::u8 curr_key_states[VKEY_SIZE_BYTE];
};

} // namespace

namespace slk {

// TODO: change kb/mouse_events container type
// TODO: consider using a single container for all type of events e.g. raw queue
// TODO: should the queue of events be thread safe
struct InputApi::State {
    MouseState m_mouse_state;
    KeyboardState m_keyboard_state;
    GestureState m_gesture_state;
    std::vector<slk::GestureEvent> m_gesture_events;
    std::vector<slk::MouseEvent> m_mouse_events;
    std::vector<slk::KeyboardEvent> m_keyboard_events;
    slk::EInputModifierMask m_modififers;
};

InputApi* InputApi::ms_instance = nullptr;

InputApi::InputApi() {
    m_state = new State{};

    m_state->m_mouse_events.reserve(128);
    m_state->m_keyboard_events.reserve(128);
    m_state->m_gesture_events.reserve(5);
}

InputApi::~InputApi() {
    delete m_state;
}


b8 InputApi::initialize() {
    assert(!ms_instance);

    ms_instance = new InputApi{};

    return true;
}

b8 InputApi::shutdown() {
    assert(ms_instance);

    delete (ms_instance->m_state);
    ms_instance->m_state = nullptr;

    return false;
}

static constexpr u8 MB_MODIFIER_OFFSET = 4U;
static constexpr u8 KB_MODIFIERS_COUNT = MB_MODIFIER_OFFSET;

static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::LEFT)) == std::to_underlying(EInputModifier::LMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::MIDDLE)) == std::to_underlying(EInputModifier::MMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::RIGHT)) == std::to_underlying(EInputModifier::RMB));

void InputApi::processEvents() {
    GestureState& gesture_state = m_state->m_gesture_state;

    u8 max_point_count = 0;
    for (GestureEvent const& evt : m_state->m_gesture_events) {
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
                if (evt.m_type == EInputEventType::GESTURE_SCROLL)
                    gesture_state.m_scroll_delta = evt.m_delta;
                else if (evt.m_type == EInputEventType::GESTURE_MAGNIFY)
                    gesture_state.m_magnify_delta = evt.m_delta;
            }
        }
    }

    MouseState& mouse_state = m_state->m_mouse_state;
    for (MouseEvent const& evt : m_state->m_mouse_events) {
        if (evt.m_type == EInputEventType::MOUSE_MOVE) {
            mouse_state.m_curr_postion = evt.m_postion;
            mouse_state.m_data_state |= MouseState::HAS_POSITION;
        } else if (evt.m_type == EInputEventType::MOUSE_BUTTON_DOWN) {
            mouse_state.m_curr_buttons_state |= static_cast<EMouseButtonMask>(1U << std::to_underlying(evt.m_button));
            m_state->m_modififers |= static_cast<EInputModifierMask>(1U << (std::to_underlying(evt.m_button) + MB_MODIFIER_OFFSET));
        } else if (evt.m_type == EInputEventType::MOUSE_BUTTON_UP) {
            mouse_state.m_curr_buttons_state &= ~static_cast<EMouseButtonMask>(1U << std::to_underlying(evt.m_button));
            m_state->m_modififers &= ~static_cast<EInputModifierMask>(1U << (std::to_underlying(evt.m_button) + MB_MODIFIER_OFFSET));
        } else if (evt.m_type == EInputEventType::MOUSE_SCROLL) {
            mouse_state.m_scroll += evt.m_scroll;
        } else {
            assert(false);
        }
    }

    u8 keyboard_modifiers[KB_MODIFIERS_COUNT] = {};
    KeyboardState& keyboard_state = m_state->m_keyboard_state;
    for (KeyboardEvent const& evt : m_state->m_keyboard_events) {
        i8 modifier_val = 1;
        if (evt.m_type == EInputEventType::KEYBOARD_KEY_DOWN) {
            u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.m_vkey);
            u8 const byte_offset = KeyboardState::getVKeyByteOffset(evt.m_vkey);

            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);
            keyboard_state.curr_key_states[byte_idx] |= 1U << byte_offset;
        } else if (evt.m_type == EInputEventType::KEYBOARD_KEY_UP) {
            u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.m_vkey);
            u8 const byte_offset = KeyboardState::getVKeyByteOffset(evt.m_vkey);
            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

            keyboard_state.curr_key_states[byte_idx] &= ~(1U << byte_offset);
            modifier_val = -1;
        } else {
            assert(false);
        }

        if (evt.m_vkey == EKeyboardVKey::LEFT_CONTROL || evt.m_vkey == EKeyboardVKey::RIGHT_CONTROL) {
            keyboard_modifiers[std::to_underlying(EInputModifier::CTRL)] += modifier_val;
        }
        if (evt.m_vkey == EKeyboardVKey::LEFT_SHIFT || evt.m_vkey == EKeyboardVKey::RIGHT_SHIFT) {
            keyboard_modifiers[std::to_underlying(EInputModifier::SHIFT)] += modifier_val;
        }
        if (evt.m_vkey == EKeyboardVKey::LEFT_SUPER || evt.m_vkey == EKeyboardVKey::RIGHT_SUPER) {
            keyboard_modifiers[std::to_underlying(EInputModifier::SUPER)] += modifier_val;
        }
        if (evt.m_vkey == EKeyboardVKey::LEFT_ALT || evt.m_vkey == EKeyboardVKey::RIGHT_ALT) {
            keyboard_modifiers[std::to_underlying(EInputModifier::ALT)] += modifier_val;
        }
    }

    for (u8 idx = 0; idx != KB_MODIFIERS_COUNT; ++idx) {
        i8 const mod = keyboard_modifiers[idx];
        if (mod < 0) {
            m_state->m_modififers &= ~static_cast<EInputModifierMask>(1 << idx);
        } else if (mod > 0) {
            m_state->m_modififers |= static_cast<EInputModifierMask>(1 << idx);
        }
    }

    m_state->m_keyboard_events.clear();
    m_state->m_mouse_events.clear();
    m_state->m_gesture_events.clear();
}

void InputApi::update() {
    assert(ms_instance);

    MouseState& mouse_state = m_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_POSITION) {
        mouse_state.m_data_state |= MouseState::HAS_PREV_POSITION;
        mouse_state.m_prev_position = mouse_state.m_curr_postion;
    }
    mouse_state.m_prev_buttons_state = mouse_state.m_curr_buttons_state;
    mouse_state.m_scroll = Vector2f::ZERO;

    m_state->m_gesture_state.m_scroll_delta = Vector2f::ZERO;
    m_state->m_gesture_state.m_magnify_delta = Vector2f::ZERO;

    processEvents();
}

EMouseButtonMask InputApi::mouseButtonsState() const {
    assert(ms_instance);
    return m_state->m_mouse_state.m_curr_buttons_state;
}

b8 InputApi::mouseButtonsDown(EMouseButtonMask buttons) const {
    assert(ms_instance);
    return (m_state->m_mouse_state.m_curr_buttons_state & buttons) == buttons;
}

b8 InputApi::mouseButtonDown(EMouseButton button) const {
    assert(ms_instance);
    return (m_state->m_mouse_state.m_curr_buttons_state & static_cast<EMouseButtonMask>(1 << std::to_underlying(button))) != EMouseButtonMask::NONE;
}

Vector2f InputApi::mousePosition() const {
    assert(ms_instance);
    return m_state->m_mouse_state.m_curr_postion;
}

Vector2f InputApi::mouseMovement() const {
    assert(ms_instance);

    MouseState& mouse_state = m_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_PREV_POSITION)
        return mouse_state.m_curr_postion - mouse_state.m_prev_position;

    return {};
}

Vector2f InputApi::mouseScroll() const {
    assert(ms_instance);
    return m_state->m_mouse_state.m_scroll;
}

Vector2f InputApi::gestureMagnify() const {
    assert(ms_instance);
    return m_state->m_gesture_state.m_magnify_delta;
}

Vector2f InputApi::gestureScroll() const {
    assert(ms_instance);
    return m_state->m_gesture_state.m_scroll_delta;
}

u8 InputApi::gestureTouchCount() const {
    assert(ms_instance);
    return m_state->m_gesture_state.m_point_count;
}

b8 InputApi::keyboardKeysDown(std::span<EKeyboardVKey const> key_codes) const
{
    for (auto key_code : key_codes)
    {
        if (!keyboardKeyDown(key_code))
            return false;
    }

    return true;
}

b8 InputApi::anyKeyboardKeyDown(std::span<EKeyboardVKey const> key_codes) const
{
    for (auto key_code : key_codes)
    {
        if (keyboardKeyDown(key_code))
            return true;
    }

    return false;
}

b8 InputApi::keyboardKeyDown(EKeyboardVKey key_code) const {
    assert(ms_instance);

    u32 const byte_idx = KeyboardState::getVKeyByteIndex(key_code);
    u8 const byte_offset = KeyboardState::getVKeyByteOffset(key_code);
    assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

    return 0 != (m_state->m_keyboard_state.curr_key_states[byte_idx] & (1U << byte_offset));
}

void InputApi::forwardEvent(InputEvent const& evt) {
    assert(ms_instance);

    if ((evt.m_type >= EInputEventType::MOUSE_BUTTON_UP) && (evt.m_type <= EInputEventType::MOUSE_SCROLL)) {
        MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
        m_state->m_mouse_events.emplace_back(spec_evt);
    } else if ((evt.m_type >= EInputEventType::KEYBOARD_KEY_UP) && (evt.m_type <= EInputEventType::KEYBOARD_KEY_DOWN)) {
        KeyboardEvent const& spec_evt = static_cast<KeyboardEvent const&>(evt);
        m_state->m_keyboard_events.emplace_back(spec_evt);
    } 
    else if ((evt.m_type >= EInputEventType::GESTURE_TOUCH_COUNT_UPDATE) && (evt.m_type <= EInputEventType::GESTURE_MAGNIFY)) {
        GestureEvent const& spec_evt = static_cast<GestureEvent const&>(evt);
        m_state->m_gesture_events.emplace_back(spec_evt);
    }
    else {
        assert(false);
    }
}

EInputModifierMask InputApi::modifiersState() const {
    assert(ms_instance);

    return m_state->m_modififers;
}

b8 InputApi::hasAnyModifier(EInputModifierMask mod_mask) const {
    assert(ms_instance);

    return EInputModifierMask::NONE != (m_state->m_modififers & mod_mask);
}

b8 InputApi::hasModifier(EInputModifier mod_key) const {
    assert(ms_instance);

    return hasAnyModifier(static_cast<EInputModifierMask>(1 << std::to_underlying(mod_key)));
}

}

