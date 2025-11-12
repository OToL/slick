#pragma once

#include <core/core.hpp>

#include <type_traits>

namespace slk {

inline constexpr f32 PI_F32 = 3.14159265f;
inline constexpr f64 PI_F64 = 3.14159265358979323846;

template <typename T>
constexpr T const& minValue(const T& lval, const T& rval) {
    return lval < rval ? lval : rval;
}

template <typename T> requires std::is_trivial_v<T> && (sizeof(T) <= sizeof(void*)*2)
constexpr T minValue(T lval, T rval) {
    return lval < rval ? lval : rval;
}

template <typename T>
constexpr T const& maxValue(const T& lval, const T& rval) {
    return lval > rval ? lval : rval;
}

template <typename T> requires std::is_trivial_v<T> && (sizeof(T) <= sizeof(void*)*2)
constexpr T maxValue(T lval, T rval) {
    return lval > rval ? lval : rval;
}

namespace literals {
    constexpr slk::f32 operator""_deg(long double degrees)
    {
        return static_cast<float>(degrees * PI_F32 / 180.0f);
    }

    constexpr slk::f32 operator""_rad(long double radians)
    {
        return static_cast<float>(radians);
    }
} // namespace literals

} // namespace slk
