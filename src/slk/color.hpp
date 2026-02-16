#pragma once

#include "core.hpp"

namespace slk {

SB_DECLARE_TAG(ABGR);
SB_DECLARE_TAG(RGBA);

// abgr color
struct ColorU32 {

    union {
        struct {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        u32 abgr;
    };

    ColorU32() = default;

    constexpr explicit ColorU32(u32 value) 
        : abgr(value)
    {
    }

    constexpr ColorU32(u8 in_a, u8 in_b, u8 in_g, u8 in_r) 
        : r(in_r)
        , g(in_g)
        , b(in_b)
        , a(in_a)
    {
    }

    constexpr ColorU32(RGBATag, u32 value) 
        : r((value & 0xFF000000U) >> 24)
        , g((value & 0x00FF0000U) >> 8)
        , b((value & 0x0000FF00U) << 8)
        , a((value & 0x000000FFU) >> 24)

    {
    }

    constexpr ColorU32(RGBATag, u8 in_r, u8 in_g, u8 in_b, u8 in_a) 
        : r(in_r)
        , g(in_g)
        , b(in_b)
        , a(in_a)
    {
    }

    static inline constexpr ColorU32 red() {
        return ColorU32{0xFF0000FFU};
    }

    static inline constexpr ColorU32 green() {
        return ColorU32{0xFF00FF00U};
    }

    static inline constexpr ColorU32 blue() {
        return ColorU32{0xFFFF0000U};
    }

    static inline constexpr ColorU32 black() {
        return ColorU32{0xFF000000U};
    }

    static inline constexpr ColorU32 white() {
        return ColorU32{0xFFFFFFFFU};
    }

    constexpr u32 rgba() const {
        return  (u32)r << 24 | (u32)g << 16 | (u32)b << 8 | (u32)a;
    }

    // named 'COLOR_<color>' instead of '<color>' because of collisions with 3rd party libraries e.g. raylib
    static const ColorU32 COLOR_RED;
    static const ColorU32 COLOR_GREEN;
    static const ColorU32 COLOR_BLUE;
    static const ColorU32 COLOR_BLACK;
    static const ColorU32 COLOR_WHITE;
};

} // namespace slk
