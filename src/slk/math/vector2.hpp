#pragma once

#include <slk/core.hpp>
#include "math.hpp"

#include <cmath>

namespace slk {

template <typename T>
struct Vector2 {
    using ScalarType = T;
    using ParamType = Vector2Param<T>;

    union {
        struct {
            ScalarType m_x;
            ScalarType m_y;
        };
        ScalarType m_values[2];
    };

    constexpr Vector2() = default;

    constexpr explicit Vector2(ScalarType val)
        : m_x(val)
        , m_y(val) { }

    constexpr Vector2(ScalarType x, ScalarType y)
        : m_x(x)
        , m_y(y) { }

    constexpr ScalarType dot(ParamType v) const {
        return m_x * v.m_x + m_y * v.m_y;
    }

    constexpr Vector2& mul(ScalarType v) {
        m_x *= v;
        m_y *= v;

        return *this;
    }

    constexpr Vector2 multiplied(ScalarType v) const {
        return {m_x * v, m_y * v};
    }

    constexpr Vector2& mul(ParamType v) {
        m_x *= v.m_x;
        m_y *= v.m_y;

        return *this;
    }

    constexpr Vector2 multiplied(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y};
    }

    constexpr Vector2& normalize() {
        auto const length_val = length();
        m_x /= length_val;
        m_y /= length_val;

        return *this;
    }

    constexpr Vector2 normalized() const {
        auto const length_val = length();
        return {
            m_x /= length_val,
            m_y /= length_val,
        };
    }

    constexpr ScalarType length() const {
        return std::sqrt(m_x * m_x + m_y * m_y);
    }

    constexpr ScalarType length2() const {
        return m_x * m_x + m_y * m_y;
    }

    constexpr ScalarType* data() {
        return &m_values[0];
    }

    constexpr ScalarType const* data() const {
        return &m_values[0];
    }

    // Operators

    constexpr Vector2& operator*=(ScalarType val) {
        m_x *= val;
        m_y *= val;

        return *this;
    }

    constexpr Vector2& operator-=(ParamType const& v) {
        m_x -= v.m_x;
        m_y -= v.m_y;
        return *this;
    }

    constexpr Vector2& operator+=(ParamType const& v) {
        m_x += v.m_x;
        m_y += v.m_y;
        return *this;
    }

    constexpr Vector2 operator-() const {
        return {-m_x, -m_y};
    }

    constexpr ScalarType operator[](u32 idx) const {
        // TODO: add assertion on idx value
        return m_values[idx];
    }

    // Constants

    static constexpr Vector2 zero() {
        return {0, 0};
    }
    static constexpr Vector2 unitY() {
        return {0, 1};
    };
    static constexpr Vector2 unitX() {
        return {1, 0};
    };
    static const Vector2 ZERO;
    static const Vector2 UNITX;
    static const Vector2 UNITY;
};

// forward declaration required by Clang and MSVC
template <>
const Vector2<f32> Vector2<f32>::ZERO;
template <>
const Vector2<f32> Vector2<f32>::UNITX;
template <>
const Vector2<f32> Vector2<f32>::UNITY;
template <>
const Vector2<i32> Vector2<i32>::ZERO;
template <>
const Vector2<i32> Vector2<i32>::UNITX;
template <>
const Vector2<i32> Vector2<i32>::UNITY;

template <typename T>
inline Vector2<T> operator*(Vector2Param<T> v1, Vector2Param<T> v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

template <typename T>
inline Vector2<T> operator*(Vector2Param<T> v, T val) {
    return {v.m_x * val, v.m_y * val};
}

template <typename T>
inline Vector2<T> operator/(Vector2Param<T> v, T val) {
    return {v.m_x / val, v.m_y / val};
}

template <typename T>
inline Vector2<T> operator+(Vector2Param<T> v, T val) {
    return {v.m_x + val, v.m_y + val};
}

template <typename T>
inline Vector2<T> operator+(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y};
}

template <typename T>
inline Vector2<T> operator-(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y};
}

template <typename T>
bool operator<(Vector2Param<T> const& v1, Vector2Param<T> const& v2) {
    return v1.m_x < v2.m_x && v1.m_y < v2.m_y;
}

template <typename T>
bool operator>(Vector2Param<T> const& v1, Vector2Param<T> const& v2) {
    return v1.m_x > v2.m_x && v1.m_y > v2.m_y;
}

// component wise min/max
template <typename T>
constexpr Vector2<T> min(Vector2Param<T> lval, Vector2Param<T> rval) {
    return {
        lval.m_x < rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y < rval.m_y ? lval.m_y : rval.m_y,
    };
}

template <typename T>
constexpr Vector2<T> max(Vector2Param<T> lval, Vector2Param<T> rval) {
    return {
        lval.m_x > rval.m_x ? lval.m_x : rval.m_x,
        lval.m_y > rval.m_y ? lval.m_y : rval.m_y,
    };
}

using Vector2i = Vector2<i32>;
using Vector2f = Vector2<f32>;

} // namespace slk
