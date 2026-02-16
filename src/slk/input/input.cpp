#include "input.hpp"
#include "events.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
struct MouseState {
    enum : slk::u8 {
        HAS_POSITION = 1 << 0,
        HAS_PREV_POSITION = 1 << 1,
    };

    slk::Vector2f prev_position;
    slk::Vector2f curr_postion;
    slk::MouseButton::Type prev_buttons_state;
    slk::MouseButton::Type curr_buttons_state;
    slk::Vector2f scroll;
    slk::u8 data_state;
};

struct KeyboardState {

    static constexpr slk::u32 VKEY_COUNT = slk::VirtualKey::_LAST;
    static constexpr slk::u32 VKEY_SIZE_BYTE = (VKEY_COUNT + 7U) >> 3U;

    static slk::u32 getVKeyByteIndex(slk::EVirtualKey key) {
        return key >> 3U;
    }

    static slk::u8 getVKeyByteOffset(slk::EVirtualKey key) {
        return key % 8;
    }

    slk::u8 curr_key_states[VKEY_SIZE_BYTE];
};

// TODO: change kb/mouse_events container type
// TODO: consider using a single container for all type of events e.g. raw queue
// TODO: should the queue of events be thread safe
struct InputAPIState {
    MouseState mouse_state;
    KeyboardState keyboard_state;
    std::vector<slk::MouseEvent> mouse_events;
    std::vector<slk::KeyboardEvent> keyboard_events;
    slk::InputModifier::Mask modififers;
}* g_state = nullptr;
} // namespace

slk::b8 slk::InputAPI::init() {
    assert(!g_state);

    g_state = new InputAPIState{};
    g_state->mouse_events.reserve(128);
    g_state->keyboard_events.reserve(128);

    return true;
}

slk::b8 slk::InputAPI::shutdown() {
    assert(g_state);

    delete (g_state);
    g_state = nullptr;

    return false;
}

static constexpr slk::u8 MB_MODIFIER_OFFSET = 4U;
static constexpr slk::u8 KB_MODIFIERS_COUNT = MB_MODIFIER_OFFSET;

static_assert((MB_MODIFIER_OFFSET + slk::EMouseButton::LEFT) == slk::InputModifier::LMB);
static_assert((MB_MODIFIER_OFFSET + slk::EMouseButton::MIDDLE) == slk::InputModifier::MMB);
static_assert((MB_MODIFIER_OFFSET + slk::EMouseButton::RIGHT) == slk::InputModifier::RMB);

void processEvents() {

    MouseState& mouse_state = g_state->mouse_state;
    for (slk::MouseEvent const& evt : g_state->mouse_events) {
        if (evt.type == slk::InputEvent::MOUSE_MOVE) {
            mouse_state.curr_postion = evt.postion;
            mouse_state.data_state |= MouseState::HAS_POSITION;
        } else if (evt.type == slk::InputEvent::MOUSE_BUTTON_DOWN) {
            mouse_state.curr_buttons_state |= (1U << evt.button);
            g_state->modififers |= 1U << (evt.button + MB_MODIFIER_OFFSET);
        } else if (evt.type == slk::InputEvent::MOUSE_BUTTON_UP) {
            mouse_state.curr_buttons_state &= ~(1U << evt.button);
            g_state->modififers &= ~(1U << (evt.button + MB_MODIFIER_OFFSET));
        } else if (evt.type == slk::InputEvent::MOUSE_SCROLL) {
            mouse_state.scroll += evt.scroll;
        } else {
            assert(false);
        }
    }

    slk::u8 keyboard_modifiers[KB_MODIFIERS_COUNT] = {};
    KeyboardState& keyboard_state = g_state->keyboard_state;
    for (slk::KeyboardEvent const& evt : g_state->keyboard_events) {
        slk::i8 modifier_val = 1;
        if (evt.type == slk::InputEvent::KEYBOARD_KEY_DOWN) 
        {
            slk::u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.vkey);
            slk::u8 const byte_offset = KeyboardState::getVKeyByteIndex(evt.vkey);

            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);
            keyboard_state.curr_key_states[byte_idx] |= 1U << byte_offset;
        } else if (evt.type == slk::InputEvent::KEYBOARD_KEY_UP) {
            slk::u32 const byte_idx = KeyboardState::getVKeyByteIndex(evt.vkey);
            slk::u8 const byte_offset = KeyboardState::getVKeyByteIndex(evt.vkey);
            assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

            keyboard_state.curr_key_states[byte_idx] &= ~(1U << byte_offset);
            modifier_val = -1;
        }
        else {
            assert(false);
        }

        if (evt.vkey == slk::VirtualKey::LEFT_CONTROL || evt.vkey == slk::VirtualKey::RIGHT_CONTROL) {
            keyboard_modifiers[slk::InputModifier::CTRL] += modifier_val;
        }
        if (evt.vkey == slk::VirtualKey::LEFT_SHIFT || evt.vkey == slk::VirtualKey::RIGHT_SHIFT) {
            keyboard_modifiers[slk::InputModifier::SHIFT] += modifier_val;
        }
        if (evt.vkey == slk::VirtualKey::LEFT_SUPER || evt.vkey == slk::VirtualKey::RIGHT_SUPER) {
            keyboard_modifiers[slk::InputModifier::SUPER] += modifier_val;
        }
        if (evt.vkey == slk::VirtualKey::LEFT_ALT || evt.vkey == slk::VirtualKey::RIGHT_ALT) {
            keyboard_modifiers[slk::InputModifier::ALT] += modifier_val;
        }
    }

    for (slk::u8 idx = 0 ; idx != KB_MODIFIERS_COUNT ; ++idx) {
        slk::i8 const mod = keyboard_modifiers[idx];
        if (mod < 0) {
            g_state->modififers &= ~(slk::InputModifier::mask(idx));
        }
        else if (mod > 0) {
            g_state->modififers |= slk::InputModifier::mask(idx);
        }
    }

    g_state->keyboard_events.clear();
    g_state->mouse_events.clear();
}


void slk::InputAPI::update() {
    assert(g_state);

    MouseState& mouse_state = g_state->mouse_state;
    if (mouse_state.data_state & MouseState::HAS_POSITION) {
        mouse_state.data_state |= MouseState::HAS_PREV_POSITION;
        mouse_state.prev_position = mouse_state.curr_postion;
    }
    mouse_state.prev_buttons_state = mouse_state.curr_buttons_state;
    mouse_state.scroll = Vector2f::ZERO;

    processEvents();
}

slk::MouseButton::Mask slk::InputAPI::getMouseButtonsState() {
    assert(g_state);
    return g_state->mouse_state.curr_buttons_state;
}

slk::b8 slk::InputAPI::isMouseButtonDown(EMouseButton button) {
    assert(g_state);
    return (g_state->mouse_state.curr_buttons_state & (1 << button)) != 0;
}

slk::Vector2f slk::InputAPI::getMousePosition() {
    assert(g_state);
    return g_state->mouse_state.curr_postion;
}

slk::Vector2f slk::InputAPI::getMouseMovement() {
    assert(g_state);

    MouseState& mouse_state = g_state->mouse_state;
    if (mouse_state.data_state & MouseState::HAS_PREV_POSITION)
        return mouse_state.curr_postion - mouse_state.prev_position;

    return {};
}

slk::Vector2f slk::InputAPI::getMouseScroll() {
    assert(g_state);
    return g_state->mouse_state.scroll;
}

slk::b8 slk::InputAPI::isKeyboardKeyDown(EVirtualKey key_code) {
    assert(g_state);

    u32 const byte_idx = KeyboardState::getVKeyByteIndex(key_code);
    u8 const byte_offset = KeyboardState::getVKeyByteIndex(key_code);
    assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

    return 0 != (g_state->keyboard_state.curr_key_states[byte_idx] & (1U << byte_offset));
}

void slk::InputAPI::forwardEvent(InputEvent const& evt) {
    assert(g_state);

    if ((evt.type >= MouseEvent::MOUSE_BUTTON_UP) && (evt.type <= MouseEvent::MOUSE_SCROLL)) {
        MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
        g_state->mouse_events.emplace_back(spec_evt);
    } else if ((evt.type >= MouseEvent::KEYBOARD_KEY_UP) && (evt.type <= MouseEvent::KEYBOARD_KEY_DOWN)) {
        KeyboardEvent const& spec_evt = static_cast<KeyboardEvent const&>(evt);
        g_state->keyboard_events.emplace_back(spec_evt);
    }
}

slk::InputModifier::Mask slk::InputAPI::getModifiersState() {
    assert(g_state);

    return g_state->modififers;
} 


slk::b8 slk::InputAPI::hasModifiers(InputModifier::Mask mod_mask) {
    assert(g_state);

    return 0 != (g_state->modififers & mod_mask);
}

slk::b8 slk::InputAPI::hasModifier(EInputModifier mod_key) {
    assert(g_state);

    return hasModifiers(InputModifier::mask(mod_key));
}



