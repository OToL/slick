#pragma once

#include "core.hpp"
#include "dispatch_tag.hpp"

namespace slk {

sb_declare_dispatch_tag(Abgr);
sb_declare_dispatch_tag(Rgba);

// abgr color (0xAABBGGRR)
struct ColorU32 {
    union {
        struct {
            u8 m_red;
            u8 m_green;
            u8 m_blue;
            u8 m_alpha;
        };
        u32 m_abgr;
    };

    constexpr ColorU32() = default;

    constexpr explicit ColorU32(u32 value)
        : m_abgr(value) { }

    constexpr ColorU32(u8 a, u8 b, u8 g, u8 r)
        : m_red(r)
        , m_green(g)
        , m_blue(b)
        , m_alpha(a) { }

    constexpr ColorU32(RgbaTag, u32 value)
        : m_red((value & 0xFF000000U) >> 24)
        , m_green((value & 0x00FF0000U) >> 8)
        , m_blue((value & 0x0000FF00U) << 8)
        , m_alpha((value & 0x000000FFU) >> 24)

    { }

    constexpr ColorU32(RgbaTag, u8 r, u8 g, u8 b, u8 a)
        : ColorU32(a, b, g, r) { }

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
        return (u32)m_red << 24 | (u32)m_green << 16 | (u32)m_blue << 8 | (u32)m_alpha;
    }

    // named 'COLOR_<color>' instead of '<color>' because of collisions with 3rd party libraries e.g. raylib
    static const ColorU32 COLOR_RED;
    static const ColorU32 COLOR_GREEN;
    static const ColorU32 COLOR_BLUE;
    static const ColorU32 COLOR_BLACK;
    static const ColorU32 COLOR_WHITE;
};

} // namespace slk
