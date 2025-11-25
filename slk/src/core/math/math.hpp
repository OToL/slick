#pragma once

#include <core/core.hpp>

namespace slk {

template <typename T>
struct Vector2;
template <typename T>
struct Vector3;
template <typename T>
struct Vector4;
template <typename T>
struct AABB2;
template <typename T>
struct Matrix2;
template <typename T>
struct Matrix3;
template <typename T>
struct Matrix4;

template<typename T>
using Vector2Param = Vector2<T>;
template<typename T>
using Vector3Param = Vector3<T>;
template<typename T>
using Vector4Param = Vector4<T> const&;
template<typename T>
using AABB2Param = AABB2<T> const&;
template<typename T>
using Matrix2Param = Matrix2<T>;
template<typename T>
using Matrix3Param = Matrix3<T> const&;
template<typename T>
using Matrix4Param = Matrix4<T> const&;

inline constexpr f32 PI_F32 = 3.14159265f;
inline constexpr f64 PI_F64 = 3.14159265358979323846;

template <typename T>
constexpr T min_value(T lval, T rval) {
    return lval < rval ? lval : rval;
}

template <typename T> 
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
