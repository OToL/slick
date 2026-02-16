#pragma once

#include <slk/core.hpp>
#include "math.hpp"

#include <cmath>

namespace slk {

template <typename T>
struct alignas(16) Vector4 {
    using ScalarType = T;
    using ParamType = Vector4Param<T>;

    union {
        struct {
            ScalarType x;
            ScalarType y;
            ScalarType z;
            ScalarType w;
        };
        ScalarType values[4];
    };

    Vector4() = default;

    explicit Vector4(ScalarType val)
        : x(val)
        , y(val)
        , z(val)
        , w(val) { }

    Vector4(ScalarType _x, ScalarType _y, ScalarType _z, ScalarType _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w) { }

    ScalarType dot(ParamType v) const {
        return x * v.x + y * v.y + w * v.w;
    }

    Vector4 cross(ParamType v) const {
		return {
			y*v.z - z*v.y,
			z*v.w - w*v.z,
			w*v.x - x*v.w,
			x*v.y - y*v.x,
		};
    }


    Vector4& mul(ScalarType v) {
        x *= v;
        y *= v;
        z *= v;
        w *= v;

        return *this;
    }

    Vector4 multiplied(ScalarType v) const {
        return {x * v, y * v, z * v, w * v};
    }

    Vector4& mul(ParamType v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        w *= v.w;

        return *this;
    }

    Vector4 multiplied(ParamType v) const {
        return {x * v.x, y * v.y, z * v.z, w * v.w};
    }

    Vector4& normalize() {
        auto const length_val = length();
        x /= length_val;
        y /= length_val;
        z /= length_val;
        w /= length_val;

        return *this;
    }

    Vector4 normalized() const {
        auto const length_val = length();
        return {
            x /= length_val,
            y /= length_val,
            z /= length_val,
            w /= length_val,
        };
    }

    ScalarType length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    ScalarType length2() const {
        return x * x + y * y + z * z + w * w;
    }

    ScalarType* data() {
        return &values[0];
    }

    ScalarType const* data() const {
        return &values[0];
    }

    // Operators

    Vector4& operator*=(ScalarType val) {
        x *= val;
        y *= val;
        z *= val;
        w *= val;

        return *this;
    }

    Vector4& operator-=(ParamType v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;

        return *this;
    }

    Vector4& operator+=(ParamType v) {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;

        return *this;
    }

    Vector4 operator-() const {
        return {-x, -y, -z, -w};
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

    static constexpr Vector4 zero() {
        return {0, 0, 0, 0};
    }
    static constexpr Vector4 unitY() {
        return {0, 1, 0, 0};
    };
    static constexpr Vector4 unitX() {
        return {1, 0, 0, 0};
    };
    static constexpr Vector4 unitZ() {
        return {0, 0, 1, 0};
    };
    static constexpr Vector4 unitW() {
        return {0, 0, 0, 1};
    };
    static const Vector4 ZERO;
    static const Vector4 UNITX;
    static const Vector4 UNITY;
    static const Vector4 UNITZ;
    static const Vector4 UNITW;
};

// forward declaration required by Clang and MSVC
template <>
const Vector4<slk::f32> Vector4<slk::f32>::ZERO;
template <>
const Vector4<slk::f32> Vector4<slk::f32>::UNITX;
template <>
const Vector4<slk::f32> Vector4<slk::f32>::UNITY;
template <>
const Vector4<slk::f32> Vector4<slk::f32>::UNITZ;
template <>
const Vector4<slk::f32> Vector4<slk::f32>::UNITW;
template <>
const Vector4<slk::i32> Vector4<slk::i32>::ZERO;
template <>
const Vector4<slk::i32> Vector4<slk::i32>::UNITX;
template <>
const Vector4<slk::i32> Vector4<slk::i32>::UNITY;
template <>
const Vector4<slk::i32> Vector4<slk::i32>::UNITZ;
template <>
const Vector4<slk::i32> Vector4<slk::i32>::UNITW;

template <typename T>
inline Vector4<T> operator*(Vector4Param<T> v1, Vector4Param<T> v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w};
}

template <typename T>
inline Vector4<T> operator*(Vector4Param<T> v, T val) {
    return {v.x * val, v.y * val, v.z * val, v.w * val};
}

template <typename T>
inline Vector4<T> operator/(Vector4Param<T> v, T val) {
    return {v.x / val, v.y / val, v.z / val, v.w / val};
}

template <typename T>
inline Vector4<T> operator+(Vector4Param<T> v, T val) {
    return {v.x + val, v.y + val, v.z + val, v.w + val};
}

template <typename T>
inline Vector4<T> operator+(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.x + val.x, v.y + val.y, v.z + val.z, v.w + val.w};
}

template <typename T>
inline Vector4<T> operator-(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.x - val.x, v.y - val.y, v.z - val.z, v.w - val.w};
}

template <typename T>
bool operator<(slk::Vector4Param<T> v1, slk::Vector4Param<T> v2) {
    return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z && v1.w < v2.w;
}

template <typename T>
bool operator>(slk::Vector4Param<T> v1, slk::Vector4Param<T> v2) {
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z && v1.w > v2.w;
}

// component wise min/max
template <typename T>
constexpr slk::Vector4<T> min(slk::Vector4Param<T> lval, slk::Vector4Param<T> rval) {
    return {
        lval.x < rval.x ? lval.x : rval.x,
        lval.y < rval.y ? lval.y : rval.y,
        lval.z < rval.z ? lval.z : rval.z,
        lval.w < rval.w ? lval.w : rval.w,
    };
}

template <typename T>
constexpr slk::Vector4<T> max(slk::Vector4Param<T> lval, slk::Vector4Param<T> rval) {
    return {
        lval.x > rval.x ? lval.x : rval.x,
        lval.y > rval.y ? lval.y : rval.y,
        lval.z > rval.z ? lval.z : rval.z,
        lval.w > rval.w ? lval.w : rval.w,
    };
}

using Vector4i = Vector4<slk::i32>;
using Vector4f = Vector4<slk::f32>;

} // namespace slk
