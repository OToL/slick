#pragma once

#include "core.hpp"

import std;

namespace slk {

// stored value: 0xAABBGGRR; in memory: bytes R, G, B, A (RGBA8)
struct [[nodiscard]] ColorU32 {
    u32 m_abgr = 0;

    static constexpr ColorU32 makeRgba(u8 r, u8 g, u8 b, u8 a) {
        return {u32(r) | u32(g) << 8 | u32(b) << 16 | u32(a) << 24};
    }

    // 0xRRGGBBAA, for 0xAABBGGRR values use ColorU32{.m_abgr = ...}
    static constexpr ColorU32 makePackedRgba(u32 rgba) {
        return {std::byteswap(rgba)};
    }

    constexpr u32 abgr() const {
        return m_abgr;
    }

    constexpr u32 rgba() const {
        return std::byteswap(m_abgr);
    }

    constexpr u8 r() const {
        return u8(m_abgr);
    }

    constexpr u8 g() const {
        return u8(m_abgr >> 8);
    }

    constexpr u8 b() const {
        return u8(m_abgr >> 16);
    }

    constexpr u8 a() const {
        return u8(m_abgr >> 24);
    }

    constexpr ColorU32 withAlpha(u8 alpha) const {
        return {(m_abgr & 0x00FFFFFFU) | u32(alpha) << 24};
    }

    constexpr b8 operator==(ColorU32 const&) const = default;

    static consteval ColorU32 RED() {
        return {.m_abgr = 0xFF0000FFU};
    }

    static consteval ColorU32 GREEN() {
        return {.m_abgr = 0xFF00FF00U};
    }

    static consteval ColorU32 BLUE() {
        return {.m_abgr = 0xFFFF0000U};
    }

    static consteval ColorU32 BLACK() {
        return {.m_abgr = 0xFF000000U};
    }

    static consteval ColorU32 WHITE() {
        return {.m_abgr = 0xFFFFFFFFU};
    }

    static consteval ColorU32 PINK() {
        return {.m_abgr = 0xFFFF00FFU};
    }
};

} // namespace slk
