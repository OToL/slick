#pragma once

#include <slk/core.hpp>

#include <assert.h>

import std;

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

// verifies a value is aglined to a powere of 2 alignment e.g. isAlignedTo(32u, 16u) == true
template <std::unsigned_integral T>
constexpr b8 isAlignedTo(T value, T alignment) {
    assert(std::has_single_bit(alignment) && "alignment must be a power of 2");
    return 0 == (value & (alignment - 1));
}

// rounds value up to the next multiple of alignment, e.g. alignUp(13u, 8u) == 16
template <std::unsigned_integral T>
constexpr T alignUp(T value, T alignment) {
    assert(std::has_single_bit(alignment) && "alignment must be a power of 2");
    return (value + alignment - 1) & ~(alignment - 1);
}

// rounds value down to the previous multiple of alignment, e.g. alignDown(13u, 8u) == 8
template <std::unsigned_integral T>
constexpr T alignDown(T value, T alignment) {
    assert(std::has_single_bit(alignment) && "alignment must be a power of 2");
    return value & ~(alignment - 1);
}

}


