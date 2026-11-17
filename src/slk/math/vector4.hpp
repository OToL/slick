#pragma once

#include <slk/core.hpp>
#include "math.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] alignas(16) Vector4 {
    using ScalarType = T;
    using ParamType = Vector4Param<T>;

    union {
        struct {
            ScalarType m_x;
            ScalarType m_y;
            ScalarType m_z;
            ScalarType m_w;
        };
        ScalarType m_values[4];
    };

    constexpr Vector4() = default;

    constexpr explicit Vector4(ScalarType val)
        : m_x(val)
        , m_y(val)
        , m_z(val)
        , m_w(val) { }

    constexpr Vector4(ScalarType x, ScalarType y, ScalarType z, ScalarType w)
        : m_x(x)
        , m_y(y)
        , m_z(z)
        , m_w(w) { }

    constexpr ScalarType dot(this ParamType self, ParamType v) {
        return self.m_x * v.m_x + self.m_y * v.m_y + self.m_z * v.m_z + self.m_w * v.m_w;
    }

    constexpr Vector4 scaled(ScalarType v) const {
        return {m_x * v, m_y * v, m_z * v, m_w * v};
    }

    constexpr Vector4 scaled(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y, m_z * v.m_z, m_w * v.m_w};
    }

    constexpr Vector4& normalize() {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
        m_x /= length_val;
        m_y /= length_val;
        m_z /= length_val;
        m_w /= length_val;

        return *this;
    }

    constexpr Vector4 normalized() const {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
        return {
            m_x / length_val,
            m_y / length_val,
            m_z / length_val,
            m_w / length_val,
        };
    }

    constexpr ScalarType length() const {
        return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
    }

    constexpr ScalarType length2() const {
        return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
    }

    constexpr ScalarType* data() {
        return &m_values[0];
    }

    constexpr ScalarType const* data() const {
        return &m_values[0];
    }

    // Component-wise comparisons i.e. true when the comparison holds for every component

    constexpr b8 allLess(ParamType v) const {
        return m_x < v.m_x && m_y < v.m_y && m_z < v.m_z && m_w < v.m_w;
    }

    constexpr b8 allLessEqual(ParamType v) const {
        return m_x <= v.m_x && m_y <= v.m_y && m_z <= v.m_z && m_w <= v.m_w;
    }

    constexpr b8 allGreater(ParamType v) const {
        return m_x > v.m_x && m_y > v.m_y && m_z > v.m_z && m_w > v.m_w;
    }

    constexpr b8 allGreaterEqual(ParamType v) const {
        return m_x >= v.m_x && m_y >= v.m_y && m_z >= v.m_z && m_w >= v.m_w;
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

    constexpr ScalarType operator[](u32 idx) const {
        assert(idx < 4 && "index out of range");
        if consteval { // reading m_values while m_x.. are the active union members is not allowed at compile time
            return idx == 0 ? m_x : idx == 1 ? m_y : idx == 2 ? m_z : m_w;
        } else {
            return m_values[idx];
        }
    }

    constexpr ScalarType& operator[](u32 idx) {
        assert(idx < 4 && "index out of range");
        if consteval {
            switch (idx) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
                default: return m_w;
            }
        } else {
            return m_values[idx];
        }
    }

    // Constants

    static consteval Vector4 ZERO() {
        return {0, 0, 0, 0};
    }
    static consteval Vector4 UNIT_X() {
        return {1, 0, 0, 0};
    }
    static consteval Vector4 UNIT_Y() {
        return {0, 1, 0, 0};
    }
    static consteval Vector4 UNIT_Z() {
        return {0, 0, 1, 0};
    }
    static consteval Vector4 UNIT_W() {
        return {0, 0, 0, 1};
    }
};

template <typename T>
constexpr Vector4<T> operator*(Vector4Param<T> v1, Vector4Param<T> v2) {
    return {v1.m_x * v2.m_x, v1.m_y * v2.m_y, v1.m_z * v2.m_z, v1.m_w * v2.m_w};
}

template <typename T>
constexpr Vector4<T> operator*(Vector4Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x * val, v.m_y * val, v.m_z * val, v.m_w * val};
}

template <typename T>
constexpr Vector4<T> operator/(Vector4Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x / val, v.m_y / val, v.m_z / val, v.m_w / val};
}

template <typename T>
constexpr Vector4<T> operator+(Vector4Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x + val, v.m_y + val, v.m_z + val, v.m_w + val};
}

template <typename T>
constexpr Vector4<T> operator+(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y, v.m_z + val.m_z, v.m_w + val.m_w};
}

template <typename T>
constexpr Vector4<T> operator-(Vector4Param<T> v, Vector4Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y, v.m_z - val.m_z, v.m_w - val.m_w};
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
