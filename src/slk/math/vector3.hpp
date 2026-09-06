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
            ScalarType m_x;
            ScalarType m_y;
            ScalarType m_z;
        };
        ScalarType m_values[3];
    };

    constexpr Vector3() = default;

    explicit constexpr Vector3(ScalarType val)
        : m_x(val)
        , m_y(val)
        , m_z(val) { }

    constexpr Vector3(ScalarType x, ScalarType y, ScalarType z)
        : m_x(x)
        , m_y(y)
        , m_z(z) { }

    constexpr ScalarType dot(ParamType v) const {
        return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z;
    }

    constexpr Vector3 cross(ParamType v) const {
		return {
			m_y*v.m_z - m_z*v.m_y,
			m_z*v.m_x - m_x*v.m_z,
			m_x*v.m_y - m_y*v.m_x,
		};
    }

    constexpr Vector3& mul(ScalarType v) {
        m_x *= v;
        m_y *= v;
        m_z *= v;

        return *this;
    }

    constexpr Vector3 multiplied(ScalarType v) const {
        return {m_x * v, m_y * v, m_z * v};
    }

    constexpr Vector3& mul(ParamType v) {
        m_x *= v.m_x;
        m_y *= v.m_y;
        m_z *= v.m_z;

        return *this;
    }

    constexpr Vector3 multiplied(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y, m_z * v.m_z};
    }

    constexpr Vector3& normalize() {
        auto const length_val = length();
        m_x /= length_val;
        m_y /= length_val;
        m_z /= length_val;

        return *this;
    }

    constexpr Vector3 normalized() const {
        auto const length_val = length();
        return {
            m_x / length_val,
            m_y / length_val,
            m_z / length_val,
        };
    }

    constexpr ScalarType length() const {
        return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
    }

    constexpr ScalarType length2() const {
        return m_x * m_x + m_y * m_y + m_z * m_z;
    }

    constexpr ScalarType* data() {
        return &m_values[0];
    }

    constexpr ScalarType const* data() const {
        return &m_values[0];
    }

    // Operators

    constexpr Vector3& operator*=(ScalarType val) {
        m_x *= val;
        m_y *= val;
        m_z *= val;

        return *this;
    }

    constexpr Vector3& operator-=(ParamType const& v) {
        m_x -= v.m_x;
        m_y -= v.m_y;
        m_z -= v.m_z;

        return *this;
    }

    constexpr Vector3& operator+=(ParamType const& v) {
        m_x += v.m_x;
        m_y += v.m_y;
        m_z += v.m_z;

        return *this;
    }

    constexpr Vector3 operator-() const {
        return {-m_x, -m_y, -m_z};
    }

    constexpr ScalarType operator[](slk::u32 idx) const {
        // TODO: add assertion on idx value
        return m_values[idx];
    }

    constexpr ScalarType& operator[](slk::u32 idx) {
        // TODO: add assertion on idx value
        return m_values[idx];
    }

    // Constants

    static constexpr Vector3 zero() {
        return {0, 0, 0};
    }
    static constexpr Vector3 unit_y() {
        return {0, 1, 0};
    };
    static constexpr Vector3 unit_x() {
        return {1, 0, 0};
    };
    static constexpr Vector3 unit_z() {
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
    return {v1.m_x * v2.m_x, v1.m_y * v2.m_y, v1.m_z * v2.m_z};
}

template <typename T>
inline Vector3<T> operator*(Vector3Param<T> v, T val) {
    return {v.m_x * val, v.m_y * val, v.m_z * val};
}

template <typename T>
inline Vector3<T> operator/(Vector3Param<T> v, T val) {
    return {v.m_x / val, v.m_y / val, v.m_z / val};
}

template <typename T>
inline Vector3<T> operator+(Vector3Param<T> v, T val) {
    return {v.m_x + val, v.m_y + val, v.m_z + val};
}

template <typename T>
inline Vector3<T> operator+(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y, v.m_z + val.m_z};
}

template <typename T>
inline Vector3<T> operator-(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y, v.m_z - val.m_z};
}

template <typename T>
inline bool operator<(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.m_x < v2.m_x && v1.m_y < v2.m_y && v1.m_z < v2.m_z;
}

template <typename T>
inline bool operator>(slk::Vector3Param<T> const& v1, slk::Vector3Param<T> const& v2) {
    return v1.m_x > v2.m_x && v1.m_y > v2.m_y && v1.m_z > v2.m_z;
}

// component wise min/max
template <typename T>
inline constexpr slk::Vector3<T> min(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.m_x < rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y < rval.m_y ? lval.m_y : rval.m_y,
        lval.m_z < rval.m_z ? lval.m_z : rval.m_z,
    };
}

template <typename T>
inline constexpr slk::Vector3<T> max(slk::Vector3Param<T> lval, slk::Vector3Param<T> rval) {
    return {
        lval.m_x > rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y > rval.m_y ? lval.m_y : rval.m_y,
        lval.m_z > rval.m_z ? lval.m_z : rval.m_z,
    };
}

using Vector3i = Vector3<slk::i32>;
using Vector3f = Vector3<slk::f32>;

} // namespace slk
