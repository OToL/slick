#pragma once

#include <slk/core.hpp>
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

    constexpr Vector3() = default;

    explicit constexpr Vector3(ScalarType val)
        : x(val)
        , y(val)
        , z(val) { }

    constexpr Vector3(ScalarType _x, ScalarType _y, ScalarType _z)
        : x(_x)
        , y(_y)
        , z(_z) { }

    ScalarType dot(ParamType v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector3 cross(ParamType v) const {
		return {
			y*v.z - z*v.y,
			z*v.x - x*v.z,
			x*v.y - y*v.x,
		};
    }

    Vector3& mul(ScalarType v) {
        x *= v;
        y *= v;
        z *= v;

        return *this;
    }

    Vector3 multiplied(ScalarType v) const {
        return {x * v, y * v, z * v};
    }

    Vector3& mul(ParamType v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    Vector3 multiplied(ParamType v) const {
        return {x * v.x, y * v.y, z * v.z};
    }

    Vector3& normalize() {
        auto const length_val = length();
        x /= length_val;
        y /= length_val;
        z /= length_val;

        return *this;
    }

    Vector3 normalized() const {
        auto const length_val = length();
        return {
            x / length_val,
            y / length_val,
            z / length_val,
        };
    }

    ScalarType length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    ScalarType length2() const {
        return x * x + y * y + z * z;
    }

    ScalarType* data() {
        return &values[0];
    }

    ScalarType const* data() const {
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

    Vector3 operator-() const {
        return {-x, -y, -z};
    }

    ScalarType operator[](slk::u32 idx) const {
        // TODO: add assertion on idx value
        return values[idx];
    }

    ScalarType& operator[](slk::u32 idx) {
        // TODO: add assertion on idx value
        return values[idx];
    }

    // Constants

    static constexpr Vector3 zero() {
        return {0, 0, 0};
    }
    static constexpr Vector3 unitY() {
        return {0, 1, 0};
    };
    static constexpr Vector3 unitX() {
        return {1, 0, 0};
    };
    static constexpr Vector3 unitZ() {
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
inline Vector3<T> operator*(Vector3Param<T> v1, Vector3Param<T> v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

template <typename T>
inline Vector3<T> operator*(Vector3Param<T> v, T val) {
    return {v.x * val, v.y * val, v.z * val};
}

template <typename T>
inline Vector3<T> operator/(Vector3Param<T> v, T val) {
    return {v.x / val, v.y / val, v.z / val};
}

template <typename T>
inline Vector3<T> operator+(Vector3Param<T> v, T val) {
    return {v.x + val, v.y + val, v.z + val};
}

template <typename T>
inline Vector3<T> operator+(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.x + val.x, v.y + val.y, v.z + val.z};
}

template <typename T>
inline Vector3<T> operator-(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.x - val.x, v.y - val.y, v.z - val.z};
}

template <typename T>
bool operator<(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z;
}

template <typename T>
bool operator>(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z;
}

// component wise min/max
template <typename T>
constexpr slk::Vector3<T> min(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.x < rval.x ? lval.x : rval.x,
        lval.y < rval.y ? lval.y : rval.y,
        lval.z < rval.z ? lval.z : rval.z,
    };
}

template <typename T>
constexpr slk::Vector3<T> max(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.x > rval.x ? lval.x : rval.x,
        lval.y > rval.y ? lval.y : rval.y,
        lval.z > rval.z ? lval.z : rval.z,
    };
}

using Vector3i = Vector3<slk::i32>;
using Vector3f = Vector3<slk::f32>;

} // namespace slk
