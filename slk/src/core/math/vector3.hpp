#pragma once

#include <core/core.hpp>
#include "math.hpp"

#include <cmath>

namespace slk {

template <typename T>
struct Vector3 {
    using ScalarType = T;
    using ParamType = Vector3Param<T>;

    union {
        struct {
            ScalarType x;
            ScalarType y;
            ScalarType z;
        };
        ScalarType values[3];
    };

    Vector3() = default;

    explicit Vector3(ScalarType val)
        : x(val)
        , y(val)
        , z(val) { }

    Vector3(ScalarType in_x, ScalarType in_y, ScalarType in_z)
        : x(in_x)
        , y(in_y)
        , z(in_z) { }

    [[nodiscard]] ScalarType dot(ParamType v) const {
        return x * v.x + y * v.y;
    }

    Vector3& mul(ScalarType v) {
        x *= v;
        y *= v;
        z *= v;

        return *this;
    }

    [[nodiscard]] Vector3 multiplied(ScalarType v) const {
        return {x * v, y * v, z * v};
    }

    Vector3& mul(ParamType v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    [[nodiscard]] Vector3 multiplied(ParamType v) const {
        return {x * v.x, y * v.y, z * v.z};
    }

    Vector3& normalize() {
        auto const length_val = length();
        x /= length_val;
        y /= length_val;
        z /= length_val;

        return *this;
    }

    [[nodiscard]] Vector3 normalized() const {
        auto const length_val = length();
        return {
            x /= length_val,
            y /= length_val,
            z /= length_val,
        };
    }

    [[nodiscard]] ScalarType length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] ScalarType length2() const {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] ScalarType* data() {
        return &values[0];
    }

    [[nodiscard]] ScalarType const* data() const {
        return &values[0];
    }

    // Operators

    Vector3& operator*=(ScalarType val) {
        x *= val;
        y *= val;
        z *= val;

        return *this;
    }

    Vector3& operator-=(ParamType const& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    Vector3& operator+=(ParamType const& v) {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    [[nodiscard]] Vector3 operator-() const {
        return {-x, -y, -z};
    }

    [[nodiscard]] ScalarType operator[](slk::u32 idx) const {
        // TODO: add assertion on idx value
        return values[idx];
    }

    // Constants

    [[nodiscard]] static constexpr Vector3 zero() {
        return {0, 0, 0};
    }
    [[nodiscard]] static constexpr Vector3 unitY() {
        return {0, 1, 0};
    };
    [[nodiscard]] static constexpr Vector3 unitX() {
        return {1, 0, 0};
    };
    [[nodiscard]] static constexpr Vector3 unitZ() {
        return {0, 0, 1};
    };
    static const Vector3 ZERO;
    static const Vector3 UNITX;
    static const Vector3 UNITY;
    static const Vector3 UNITZ;
};

// forward declaration required by Clang and MSVC
template <>
const Vector3<slk::f32> Vector3<slk::f32>::ZERO;
template <>
const Vector3<slk::f32> Vector3<slk::f32>::UNITX;
template <>
const Vector3<slk::f32> Vector3<slk::f32>::UNITY;
template <>
const Vector3<slk::f32> Vector3<slk::f32>::UNITZ;
template <>
const Vector3<slk::i32> Vector3<slk::i32>::ZERO;
template <>
const Vector3<slk::i32> Vector3<slk::i32>::UNITX;
template <>
const Vector3<slk::i32> Vector3<slk::i32>::UNITY;
template <>
const Vector3<slk::i32> Vector3<slk::i32>::UNITZ;

template <typename T>
[[nodiscard]] inline Vector3<T> operator*(Vector3Param<T> v1, Vector3Param<T> v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

template <typename T>
[[nodiscard]] inline Vector3<T> operator*(Vector3Param<T> v, T val) {
    return {v.x * val, v.y * val, v.z * val};
}

template <typename T>
[[nodiscard]] inline Vector3<T> operator/(Vector3Param<T> v, T val) {
    return {v.x / val, v.y / val, v.z / val};
}

template <typename T>
[[nodiscard]] inline Vector3<T> operator+(Vector3Param<T> v, T val) {
    return {v.x + val, v.y + val, v.z + val};
}

template <typename T>
[[nodiscard]] inline Vector3<T> operator+(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.x + val.x, v.y + val.y, v.z + val.z};
}

template <typename T>
[[nodiscard]] inline Vector3<T> operator-(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.x - val.x, v.y - val.y, v.z - val.z};
}

template <typename T>
[[nodiscard]] bool operator<(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z;
}

template <typename T>
[[nodiscard]] bool operator>(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z;
}

template <typename T>
[[nodiscard]] constexpr slk::Vector3<T> min_components(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.x < rval.x ? lval.x : rval.x,
        lval.y < rval.y ? lval.y : rval.y,
        lval.z < rval.z ? lval.z : rval.z,
    };
}

template <typename T>
[[nodiscard]] constexpr slk::Vector3<T> max_components(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.x > rval.x ? lval.x : rval.x,
        lval.y > rval.y ? lval.y : rval.y,
        lval.z > rval.z ? lval.z : rval.z,
    };
}

using Vector3i = Vector3<slk::i32>;
using Vector3f = Vector3<slk::f32>;

} // namespace slk
