#include "input_api.hpp"
#include "events.hpp"
#include <slk/math/utils.hpp>
#include <slk/api.hpp>
#include <slk/std/bit.hpp>

#include <cassert>
#include <cstdlib>
#include <cstring>

import std;

namespace slk {

struct MouseState {
    static constexpr u8 HAS_POSITION = 1 << 0;
    static constexpr u8 HAS_PREV_POSITION = 1 << 1;

    Vector2f m_prev_position;
    Vector2f m_curr_postion;
    EMouseButtonBits m_prev_buttons_state;
    EMouseButtonBits m_curr_buttons_state;
    Vector2f m_scroll;
    u8 m_data_state;
};

struct GestureState {
    u8 m_point_count;
    Vector2f m_scroll_delta;
    Vector2f m_magnify_delta;
};

struct KeyboardState {
    static constexpr u32 VKEY_COUNT = std::to_underlying(EKeyboardVKey::_COUNT);
    static constexpr u32 VKEY_SIZE_BYTE = (VKEY_COUNT + 7U) >> 3U;

    static u32 getVKeyByteIndex(EKeyboardVKey key) {
        return std::to_underlying(key) >> 3U;
    }

    static u8 getVKeyByteOffset(EKeyboardVKey key) {
        return std::to_underlying(key) % 8;
    }

    u8 curr_key_states[VKEY_SIZE_BYTE];
};

constexpr size_t inputEventSize(EInputEventType type) {
    switch (type) {
        case EInputEventType::MOUSE_BUTTON_UP:
        case EInputEventType::MOUSE_BUTTON_DOWN:
        case EInputEventType::MOUSE_MOVE:
        case EInputEventType::MOUSE_SCROLL: {
            return sizeof(MouseEvent);
        }
        case EInputEventType::KEYBOARD_KEY_UP:
        case EInputEventType::KEYBOARD_KEY_DOWN: {
            return sizeof(KeyboardEvent);
        }
        case EInputEventType::GESTURE_TOUCH_COUNT_UPDATE:
        case EInputEventType::GESTURE_SCROLL:
        case EInputEventType::GESTURE_MAGNIFY: {
            return sizeof(GestureEvent);
        }
        case EInputEventType::_COUNT:
            std::unreachable();
    }
}

struct EventsQueue {
    void* m_data;
    void* m_end;
    size_t m_used;

    static constexpr size_t INIT_CAPACITY = 128 * 1024;

    struct Iterator {
        void* m_data;

        InputEvent& operator*() const {
            return *static_cast<InputEvent*>(m_data);
        }

        Iterator& operator++() {
            InputEvent& evt = *static_cast<InputEvent*>(m_data);
            m_data = static_cast<u8*>(m_data) + alignUp(inputEventSize(evt.m_type), 16uz);

            return *this;
        }

        bool operator==(Iterator const& val) const {
            return m_data == val.m_data;
        }
    };

    using iterator = Iterator;
    using const_iterator = Iterator;

    EventsQueue() {
        m_data = std::malloc(INIT_CAPACITY);
        assert(m_data);

        m_end = static_cast<u8*>(m_data) + INIT_CAPACITY;
        m_used = 0;
    }

    ~EventsQueue() {
        std::free(m_data);
    }

    EventsQueue(EventsQueue const&) = delete;
    EventsQueue& operator=(EventsQueue const&) = delete;

    iterator begin() {
        return {
            .m_data = m_data,
        };
    }

    iterator end() {
        return {
            .m_data = static_cast<u8*>(m_data) + m_used};
    }

    size_t capacity() const {
        return static_cast<u8*>(m_end) - static_cast<u8*>(m_data);
    }

    void push(InputEvent const& evt) {
        size_t const evt_size = inputEventSize(evt.m_type);
        size_t const padded_evt_size = alignUp(evt_size, 16uz);

        if ((m_used + padded_evt_size) > capacity()) {
            size_t const new_capacity = capacity() + INIT_CAPACITY / 2;
            void* const new_data = std::malloc(new_capacity);
            assert(new_data);

            memcpy(new_data, m_data, m_used);

            std::free(m_data);
            m_data = new_data;
            m_end = static_cast<u8*>(m_data) + new_capacity;
        }

        assert((m_used + padded_evt_size) <= capacity());

        void* const curr_data = static_cast<u8*>(m_data) + m_used;
        memcpy(curr_data, &evt, evt_size);
        m_used += padded_evt_size;
    }

    void clear() {
        m_used = 0;
    }
};

// TODO: change kb/mouse_events container type
// TODO: consider using a single container for all type of events e.g. raw queue
// TODO: should the queue of events be thread safe
struct InputApi::State {
    MouseState m_mouse_state;
    KeyboardState m_keyboard_state;
    GestureState m_gesture_state;
    EventsQueue m_evt_queue;
    slk::EInputModifierBits m_modifiers;
};

static constexpr u8 MB_MODIFIER_OFFSET = 4U;
static constexpr u8 KB_MODIFIERS_COUNT = MB_MODIFIER_OFFSET;

static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::LEFT)) == std::to_underlying(EInputModifier::LMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::MIDDLE)) == std::to_underlying(EInputModifier::MMB));
static_assert((MB_MODIFIER_OFFSET + std::to_underlying(EMouseButton::RIGHT)) == std::to_underlying(EInputModifier::RMB));

namespace {

    void processEvents(InputApi::State& state) {
        GestureState& gesture_state = state.m_gesture_state;

        u8 keyboard_modifiers[KB_MODIFIERS_COUNT] = {};
        u8 max_point_count = 0;

        MouseState& mouse_state = state.m_mouse_state;

        for (auto& evt : state.m_evt_queue) {
            switch (evt.m_type) {
                case EInputEventType::MOUSE_BUTTON_UP: {
                    MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
                    mouse_state.m_curr_buttons_state &= ~static_cast<EMouseButtonBits>(1U << std::to_underlying(spec_evt.m_button));
                    state.m_modifiers &= ~static_cast<EInputModifierBits>(1U << (std::to_underlying(spec_evt.m_button) + MB_MODIFIER_OFFSET));
                    break;
                }
                case EInputEventType::MOUSE_BUTTON_DOWN: {
                    MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
                    mouse_state.m_curr_buttons_state |= static_cast<EMouseButtonBits>(1U << std::to_underlying(spec_evt.m_button));
                    state.m_modifiers |= static_cast<EInputModifierBits>(1U << (std::to_underlying(spec_evt.m_button) + MB_MODIFIER_OFFSET));
                    break;
                }
                case EInputEventType::MOUSE_MOVE: {
                    MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
                    mouse_state.m_curr_postion = spec_evt.m_postion;
                    mouse_state.m_data_state |= MouseState::HAS_POSITION;

                    break;
                }
                case EInputEventType::MOUSE_SCROLL: {
                    MouseEvent const& spec_evt = static_cast<MouseEvent const&>(evt);
                    mouse_state.m_scroll += spec_evt.m_scroll;
                    break;
                }
                case EInputEventType::KEYBOARD_KEY_UP:
                case EInputEventType::KEYBOARD_KEY_DOWN: {
                    KeyboardState& keyboard_state = state.m_keyboard_state;
                    KeyboardEvent const& spec_evt = static_cast<KeyboardEvent const&>(evt);

                    i8 modifier_val = 1;
                    if (evt.m_type == EInputEventType::KEYBOARD_KEY_DOWN) {
                        u32 const byte_idx = KeyboardState::getVKeyByteIndex(spec_evt.m_vkey);
                        u8 const byte_offset = KeyboardState::getVKeyByteOffset(spec_evt.m_vkey);

                        assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);
                        keyboard_state.curr_key_states[byte_idx] |= 1U << byte_offset;
                    } else if (evt.m_type == EInputEventType::KEYBOARD_KEY_UP) {
                        u32 const byte_idx = KeyboardState::getVKeyByteIndex(spec_evt.m_vkey);
                        u8 const byte_offset = KeyboardState::getVKeyByteOffset(spec_evt.m_vkey);
                        assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

                        keyboard_state.curr_key_states[byte_idx] &= ~(1U << byte_offset);
                        modifier_val = -1;
                    } else {
                        assert(false);
                    }

                    if (spec_evt.m_vkey == EKeyboardVKey::LEFT_CONTROL || spec_evt.m_vkey == EKeyboardVKey::RIGHT_CONTROL) {
                        keyboard_modifiers[std::to_underlying(EInputModifier::CTRL)] += modifier_val;
                    }
                    if (spec_evt.m_vkey == EKeyboardVKey::LEFT_SHIFT || spec_evt.m_vkey == EKeyboardVKey::RIGHT_SHIFT) {
                        keyboard_modifiers[std::to_underlying(EInputModifier::SHIFT)] += modifier_val;
                    }
                    if (spec_evt.m_vkey == EKeyboardVKey::LEFT_SUPER || spec_evt.m_vkey == EKeyboardVKey::RIGHT_SUPER) {
                        keyboard_modifiers[std::to_underlying(EInputModifier::SUPER)] += modifier_val;
                    }
                    if (spec_evt.m_vkey == EKeyboardVKey::LEFT_ALT || spec_evt.m_vkey == EKeyboardVKey::RIGHT_ALT) {
                        keyboard_modifiers[std::to_underlying(EInputModifier::ALT)] += modifier_val;
                    }
                    
                    break;
                }
                case EInputEventType::GESTURE_TOUCH_COUNT_UPDATE:
                case EInputEventType::GESTURE_SCROLL:
                case EInputEventType::GESTURE_MAGNIFY: {
                    GestureEvent const& spec_evt = static_cast<GestureEvent const&>(evt);

                    // end of all movements
                    if (spec_evt.m_point_count == 0) {
                        gesture_state.m_point_count = 0;
                        break;
                    } else {
                        // priority to the gesture with most touches
                        max_point_count = slk::max(spec_evt.m_point_count, max_point_count);
                        gesture_state.m_point_count = max_point_count;

                        // apply the gesture effect only if this is the current one
                        if (gesture_state.m_point_count == max_point_count) {
                            if (evt.m_type == EInputEventType::GESTURE_SCROLL)
                                gesture_state.m_scroll_delta = spec_evt.m_delta;
                            else if (evt.m_type == EInputEventType::GESTURE_MAGNIFY)
                                gesture_state.m_magnify_delta = spec_evt.m_delta;
                        }
                    }

                    break;
                }
                case EInputEventType::_COUNT:
                    std::unreachable();
            }
        }

        for (u8 idx = 0; idx != KB_MODIFIERS_COUNT; ++idx) {
            i8 const mod = keyboard_modifiers[idx];
            if (mod < 0) {
                state.m_modifiers &= ~static_cast<EInputModifierBits>(1 << idx);
            } else if (mod > 0) {
                state.m_modifiers |= static_cast<EInputModifierBits>(1 << idx);
            }
        }

        state.m_evt_queue.clear();
    }

} // namespace

InputApi::InputApi()
    : m_state(new State{}) {
}

InputApi::~InputApi() {
    assert(this != tryApi<InputApi>());

    delete m_state;
}

b8 InputApi::initialize() {
    return true;
}

b8 InputApi::shutdown() {
    return true;
}

void InputApi::update() {
    MouseState& mouse_state = m_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_POSITION) {
        mouse_state.m_data_state |= MouseState::HAS_PREV_POSITION;
        mouse_state.m_prev_position = mouse_state.m_curr_postion;
    }
    mouse_state.m_prev_buttons_state = mouse_state.m_curr_buttons_state;
    mouse_state.m_scroll = Vector2f::ZERO;

    m_state->m_gesture_state.m_scroll_delta = Vector2f::ZERO;
    m_state->m_gesture_state.m_magnify_delta = Vector2f::ZERO;

    processEvents(*m_state);
}

EMouseButtonBits InputApi::mouseButtonsDown() const {
    return m_state->m_mouse_state.m_curr_buttons_state;
}

Vector2f InputApi::mousePosition() const {
    return m_state->m_mouse_state.m_curr_postion;
}

Vector2f InputApi::mouseMovement() const {
    MouseState& mouse_state = m_state->m_mouse_state;
    if (mouse_state.m_data_state & MouseState::HAS_PREV_POSITION)
        return mouse_state.m_curr_postion - mouse_state.m_prev_position;

    return {};
}

Vector2f InputApi::mouseScroll() const {
    return m_state->m_mouse_state.m_scroll;
}

Vector2f InputApi::gestureMagnify() const {
    return m_state->m_gesture_state.m_magnify_delta;
}

Vector2f InputApi::gestureScroll() const {
    return m_state->m_gesture_state.m_scroll_delta;
}

u8 InputApi::gestureTouchCount() const {
    return m_state->m_gesture_state.m_point_count;
}

b8 InputApi::allKeyboardKeysDown(std::span<EKeyboardVKey const> key_codes) const {
    for (auto key_code : key_codes) {
        if (!isKeyboardKeyDown(key_code))
            return false;
    }

    return true;
}

b8 InputApi::anyKeyboardKeyDown(std::span<EKeyboardVKey const> key_codes) const {
    for (auto key_code : key_codes) {
        if (isKeyboardKeyDown(key_code))
            return true;
    }

    return false;
}

b8 InputApi::isKeyboardKeyDown(EKeyboardVKey key_code) const {
    u32 const byte_idx = KeyboardState::getVKeyByteIndex(key_code);
    u8 const byte_offset = KeyboardState::getVKeyByteOffset(key_code);
    assert(byte_idx < KeyboardState::VKEY_SIZE_BYTE);

    return 0 != (m_state->m_keyboard_state.curr_key_states[byte_idx] & (1U << byte_offset));
}

void InputApi::forwardEvent(InputEvent const& evt) {
    m_state->m_evt_queue.push(evt);
}

EInputModifierBits InputApi::modifiersDown() const {
    return m_state->m_modifiers;
}

} // namespace slk
