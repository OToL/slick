#pragma once

#include <cstdint>

namespace slk {

using u8 = std::uint8_t;
using i8 = std::int8_t;

using u16 = std::uint16_t;
using i16 = std::uint16_t;

using u32 = std::uint32_t;
using i32 = std::int32_t;

using u64 = std::uint64_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using b8 = bool;

} // namespace slk

// Compile Time Features
#define SB_FEATURE_ENABLED 2
#define SB_FEATURE_DISABLED 1

#define sb_feature_enabled(name) \
    2 == ((SB_FEATURE_##name+1) / SB_FEATURE_##name)

#define sb_feature_disabled(name) \
    2 != ((SB_FEATURE_##name+1) / SB_FEATURE_##name)

// Enumeration used as mask
#define sb_declare_enum_mask_operators(flag)                                                                                                         \
    inline constexpr flag operator|(flag lval, flag rval) {                                                                                          \
        return (flag)(static_cast<__underlying_type(flag)>(lval) | static_cast<__underlying_type(flag)>(rval));                                      \
    }                                                                                                                                                \
    inline constexpr flag operator&(flag lval, flag rval) {                                                                                          \
        return (flag)(static_cast<__underlying_type(flag)>(lval) & static_cast<__underlying_type(flag)>(rval));                                      \
    }                                                                                                                                                \
    inline constexpr void operator|=(flag& lval, flag rval) {                                                                                        \
        lval = lval | rval;                                                                                                                          \
    }                                                                                                                                                \
    inline constexpr void operator&=(flag& lval, flag rval) {                                                                                        \
        lval = lval & rval;                                                                                                                          \
    }                                                                                                                                                \
    inline constexpr flag operator~(flag val) {                                                                                                      \
        return (flag)(~static_cast<__underlying_type(flag)>(val));                                                                                   \
    }

// Toolchain specific macros
#if defined(__clang__)
#    define sb_force_inline [[clang::always_inline]]
#else
#    define sb_force_inline
#endif

