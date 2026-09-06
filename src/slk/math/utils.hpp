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

constexpr inline float toRadians(float deg)
{
    return deg * PI_F32 / 180.0f;
}

constexpr inline float toDegrees(float rad)
{
    return rad * 180.0f / PI_F32;
}

namespace literals {
    constexpr slk::f32 operator""_deg(long double degrees)
    {
        return toRadians(degrees);
    }

    constexpr slk::f32 operator""_rad(long double radians)
    {
        return static_cast<float>(radians);
    }
} // namespace literals


}
