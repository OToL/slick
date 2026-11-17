#pragma once

#include <slk/core.hpp>
#include "math.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] Vector2 {
    using ScalarType = T;
    using ParamType = Vector2<T>;

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

    constexpr ScalarType dot(this ParamType self, ParamType v) {
        return self.m_x * v.m_x + self.m_y * v.m_y;
    }

    constexpr Vector2 scaled(ScalarType v) const {
        return {m_x * v, m_y * v};
    }

    constexpr Vector2 scaled(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y};
    }

    constexpr Vector2& normalize() {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
        m_x /= length_val;
        m_y /= length_val;

        return *this;
    }

    constexpr Vector2 normalized() const {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
        return {
            m_x / length_val,
            m_y / length_val,
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

    // Component-wise comparisons i.e. true when the comparison holds for every component

    constexpr b8 allLess(ParamType v) const {
        return m_x < v.m_x && m_y < v.m_y;
    }

    constexpr b8 allLessEqual(ParamType v) const {
        return m_x <= v.m_x && m_y <= v.m_y;
    }

    constexpr b8 allGreater(ParamType v) const {
        return m_x > v.m_x && m_y > v.m_y;
    }

    constexpr b8 allGreaterEqual(ParamType v) const {
        return m_x >= v.m_x && m_y >= v.m_y;
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
        assert(idx < 2 && "index out of range");
        if consteval { // reading m_values while m_x.. are the active union members is not allowed at compile time
            return idx == 0 ? m_x : m_y;
        } else {
            return m_values[idx];
        }
    }

    constexpr ScalarType& operator[](u32 idx) {
        assert(idx < 2 && "index out of range");
        if consteval {
            switch (idx) {
                case 0: return m_x;
                default: return m_y;
            }
        } else {
            return m_values[idx];
        }
    }

    // Constants

    static consteval Vector2 ZERO() {
        return {0, 0};
    }
    static consteval Vector2 UNIT_Y() {
        return {0, 1};
    };
    static consteval Vector2 UNIT_X() {
        return {1, 0};
    };
};

template <typename T>
constexpr Vector2<T> operator*(Vector2Param<T> v1, Vector2Param<T> v2) {
    return {v1.m_x * v2.m_x, v1.m_y * v2.m_y};
}

template <typename T>
constexpr Vector2<T> operator*(Vector2Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x * val, v.m_y * val};
}

template <typename T>
constexpr Vector2<T> operator/(Vector2Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x / val, v.m_y / val};
}

template <typename T>
constexpr Vector2<T> operator+(Vector2Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x + val, v.m_y + val};
}

template <typename T>
constexpr Vector2<T> operator+(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y};
}

template <typename T>
constexpr Vector2<T> operator-(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y};
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
