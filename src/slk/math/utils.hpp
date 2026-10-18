#pragma once

#include "math.hpp"

namespace slk {

template <typename T>
constexpr inline T min(T lval, T rval) {
    return lval < rval ? lval : rval;
}

template <typename T>
constexpr inline T max(T lval, T rval) {
    return lval > rval ? lval : rval;
}

constexpr inline f32 toRadians(f32 deg) {
    return deg * PI_F32 / 180.0f;
}

constexpr inline f32 toDegrees(f32 rad) {
    return rad * 180.0f / PI_F32;
}

constexpr inline b8 nearZero(f32 val) { 
    return val <= EPSILON_F32 && val >= -EPSILON_F32;
}

namespace literals {
    constexpr slk::f32 operator""_deg(long double degrees) {
        return toRadians(degrees);
    }

    constexpr slk::f32 operator""_rad(long double radians) {
        return static_cast<float>(radians);
    }
} // namespace literals

} // namespace slk
