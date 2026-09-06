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
            ScalarType m_x;
            ScalarType m_y;
            ScalarType m_z;
            ScalarType m_w;
        };
        ScalarType values[4];
    };

    constexpr Vector4() = default;

    explicit Vector4(ScalarType val)
        : m_x(val)
        , m_y(val)
        , m_z(val)
        , m_w(val) { }

    constexpr Vector4(ScalarType x, ScalarType y, ScalarType z, ScalarType w)
        : m_x(x)
        , m_y(y)
        , m_z(z)
        , m_w(w) { }

    constexpr ScalarType dot(ParamType v) const {
        return m_x * v.m_x + m_y * v.m_y + m_w * v.m_w;
    }

    constexpr Vector4 cross(ParamType v) const {
		return {
			m_y*v.m_z - m_z*v.m_y,
			m_z*v.m_w - m_w*v.m_z,
			m_w*v.m_x - m_x*v.m_w,
			m_x*v.m_y - m_y*v.m_x,
		};
    }

    constexpr Vector4& mul(ScalarType v) {
        m_x *= v;
        m_y *= v;
        m_z *= v;
        m_w *= v;

        return *this;
    }

    constexpr Vector4 multiplied(ScalarType v) const {
        return {m_x * v, m_y * v, m_z * v, m_w * v};
    }

    constexpr Vector4& mul(ParamType v) {
        m_x *= v.m_x;
        m_y *= v.m_y;
        m_z *= v.m_z;
        m_w *= v.m_w;

        return *this;
    }

    constexpr Vector4 multiplied(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y, m_z * v.m_z, m_w * v.m_w};
    }

    constexpr Vector4& normalize() {
        auto const length_val = length();
        m_x /= length_val;
        m_y /= length_val;
        m_z /= length_val;
        m_w /= length_val;

        return *this;
    }

    constexpr Vector4 normalized() const {
        auto const length_val = length();
        return {
            m_x /= length_val,
            m_y /= length_val,
            m_z /= length_val,
            m_w /= length_val,
        };
    }

    constexpr ScalarType length() const {
        return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
    }

    constexpr ScalarType length2() const {
        return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
    }

    constexpr ScalarType* data() {
        return &values[0];
    }

    constexpr ScalarType const* data() const {
        return &values[0];
    }

    // Operators

    constexpr Vector4& operator*=(ScalarType val) {
        m_x *= val;
        m_y *= val;
        m_z *= val;
        m_w *= val;

        return *this;
    }

    constexpr Vector4& operator-=(ParamType v) {
        m_x -= v.m_x;
        m_y -= v.m_y;
        m_z -= v.m_z;
        m_w -= v.m_w;

        return *this;
    }

    constexpr Vector4& operator+=(ParamType v) {
        m_x += v.m_x;
        m_y += v.m_y;
        m_z += v.m_z;
        m_w += v.m_w;

        return *this;
    }

    constexpr Vector4 operator-() const {
        return {-m_x, -m_y, -m_z, -m_w};
    }

    constexpr ScalarType operator[](slk::u32 idx) const {
        // TODO: add assertion on idx value
        return values[idx];
    }

    constexpr ScalarType& operator[](slk::u32 idx) {
        // TODO: add assertion on idx value
        return values[idx];
    }

    // Constants

    static constexpr Vector4 zero() {
        return {0, 0, 0, 0};
    }
    static constexpr Vector4 unit_y() {
        return {0, 1, 0, 0};
    };
    static constexpr Vector4 unit_x() {
        return {1, 0, 0, 0};
    };
    static constexpr Vector4 unit_z() {
        return {0, 0, 1, 0};
    };
    static constexpr Vector4 unit_W() {
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
    return {v1.m_x * v2.m_x, v1.m_y * v2.m_y, v1.m_z * v2.m_z, v1.m_w * v2.m_w};
}

template <typename T>
inline Vector4<T> operator*(Vector4Param<T> v, T val) {
    return {v.m_x * val, v.m_y * val, v.m_z * val, v.m_w * val};
}

template <typename T>
inline Vector4<T> operator/(Vector4Param<T> v, T val) {
    return {v.m_x / val, v.m_y / val, v.m_z / val, v.m_w / val};
}

template <typename T>
inline Vector4<T> operator+(Vector4Param<T> v, T val) {
    return {v.m_x + val, v.m_y + val, v.m_z + val, v.m_w + val};
}

template <typename T>
inline Vector4<T> operator+(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y, v.m_z + val.m_z, v.m_w + val.m_w};
}

template <typename T>
inline Vector4<T> operator-(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y, v.m_z - val.m_z, v.m_w - val.m_w};
}

template <typename T>
inline bool operator<(slk::Vector4Param<T> v1, slk::Vector4Param<T> v2) {
    return v1.m_x < v2.m_x && v1.m_y < v2.m_y && v1.m_z < v2.m_z && v1.m_w < v2.m_w;
}

template <typename T>
inline bool operator>(slk::Vector4Param<T> v1, slk::Vector4Param<T> v2) {
    return v1.m_x > v2.m_x && v1.m_y > v2.m_y && v1.m_z > v2.m_z && v1.m_w > v2.m_w;
}

// component wise min/max
template <typename T>
inline constexpr slk::Vector4<T> min(slk::Vector4Param<T> lval, slk::Vector4Param<T> rval) {
    return {
        lval.m_x < rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y < rval.m_y ? lval.m_y : rval.m_y,
        lval.m_z < rval.m_z ? lval.m_z : rval.m_z,
        lval.m_w < rval.m_w ? lval.m_w : rval.m_w,
    };
}

template <typename T>
inline constexpr slk::Vector4<T> max(slk::Vector4Param<T> lval, slk::Vector4Param<T> rval) {
    return {
        lval.m_x > rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y > rval.m_y ? lval.m_y : rval.m_y,
        lval.m_z > rval.m_z ? lval.m_z : rval.m_z,
        lval.m_w > rval.m_w ? lval.m_w : rval.m_w,
    };
}

using Vector4i = Vector4<slk::i32>;
using Vector4f = Vector4<slk::f32>;

} // namespace slk
