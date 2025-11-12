#pragma once

#include "core.hpp"

namespace slk {

struct ColorU32
{
    slk::u8 r;
    slk::u8 g;
    slk::u8 b;
    slk::u8 a;

    static inline constexpr ColorU32 red() {
        return { 255u, 0u, 0u, 255u };
    }

    static inline constexpr ColorU32 green() {
        return { 0u, 255u, 0u, 255u };
    } 

    static inline constexpr ColorU32 blue() {
        return { 0u, 255u, 0u, 255u };
    } 
};

}
