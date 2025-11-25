#pragma once

#include <cstdint>

namespace slk {

using u8 = std::uint8_t;
using i8 = std::int8_t;

using u16 = std::uint16_t;
using i16 = std::uint16_t;

using u32 = std::uint32_t;
using i32 = std::int32_t;

using f32 = float;
using f64 = double;

using b8 = bool;

} // namespace slk

// requires inclusion of std utility (to_underlying)
#define DECLARE_ENUM_MASK(enum_type) \
    inline enum_type operator|(enum_type lval, enum_type rval) {                                \
        return static_cast<enum_type>(std::to_underlying(lval) | std::to_underlying(rval));     \
    }                                                                                           \
    inline enum_type operator&(enum_type lval, enum_type rval) {                                \
        return static_cast<enum_type>(std::to_underlying(lval) & std::to_underlying(rval));     \
    }                                                                                           \
    inline enum_type& operator|=(enum_type& lval, enum_type rval) {                             \
        lval = lval | rval;                                                                     \
        return lval;                                                                            \
    }                                                                                           \
    inline enum_type operator&=(enum_type& lval, enum_type rval) {                              \
        lval = lval & rval;                                                                     \
        return lval;                                                                            \
    }                                                                                           \
    inline bool has_flag(enum_type lval, enum_type rval) {                                      \
        return (lval & rval) == rval;                                                           \
    }

