#pragma once

#include <slk/core.hpp>

namespace slk {

// scrambles a 64-bit number so that every input bit affects every output bit
inline constexpr u64 scrambleBits(u64 x) {
    x ^= x >> 32;
    x *= 0xe9846af9b1a615dULL;
    x ^= x >> 32;
    x *= 0xe9846af9b1a615dULL;
    x ^= x >> 28;
    return x;
}

}


