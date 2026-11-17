#pragma once

#include <slk/core.hpp>
#include "math.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] Vector3 {
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

    constexpr ScalarType dot(this ParamType self, ParamType v) {
        return self.m_x * v.m_x + self.m_y * v.m_y + self.m_z * v.m_z;
    }

    constexpr Vector3 cross(ParamType v) const {
		return {
			m_y*v.m_z - m_z*v.m_y,
			m_z*v.m_x - m_x*v.m_z,
			m_x*v.m_y - m_y*v.m_x,
		};
    }

    constexpr Vector3 scaled(ScalarType v) const {
        return {m_x * v, m_y * v, m_z * v};
    }

    constexpr Vector3 scaled(ParamType v) const {
        return {m_x * v.m_x, m_y * v.m_y, m_z * v.m_z};
    }

    constexpr Vector3& normalize() {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
        m_x /= length_val;
        m_y /= length_val;
        m_z /= length_val;

        return *this;
    }

    constexpr Vector3 normalized() const {
        auto const length_val = length();
        assert(length_val != 0 && "normalizing a zero-length vector");
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

    // Component-wise comparisons i.e. true when the comparison holds for every component

    constexpr b8 allLess(ParamType v) const {
        return m_x < v.m_x && m_y < v.m_y && m_z < v.m_z;
    }

    constexpr b8 allLessEqual(ParamType v) const {
        return m_x <= v.m_x && m_y <= v.m_y && m_z <= v.m_z;
    }

    constexpr b8 allGreater(ParamType v) const {
        return m_x > v.m_x && m_y > v.m_y && m_z > v.m_z;
    }

    constexpr b8 allGreaterEqual(ParamType v) const {
        return m_x >= v.m_x && m_y >= v.m_y && m_z >= v.m_z;
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

    constexpr ScalarType operator[](u32 idx) const {
        assert(idx < 3 && "index out of range");
        if consteval { // reading m_values while m_x.. are the active union members is not allowed at compile time
            return idx == 0 ? m_x : idx == 1 ? m_y : m_z;
        } else {
            return m_values[idx];
        }
    }

    constexpr ScalarType& operator[](u32 idx) {
        assert(idx < 3 && "index out of range");
        if consteval {
            switch (idx) {
                case 0: return m_x;
                case 1: return m_y;
                default: return m_z;
            }
        } else {
            return m_values[idx];
        }
    }

    // Constants

    static consteval Vector3 ZERO() {
        return {0, 0, 0};
    }
    static consteval Vector3 UNIT_X() {
        return {1, 0, 0};
    }
    static consteval Vector3 UNIT_Y() {
        return {0, 1, 0};
    }
    static consteval Vector3 UNIT_Z() {
        return {0, 0, 1};
    }
};

template <typename T>
constexpr Vector3<T> operator*(Vector3Param<T> v1, Vector3Param<T> v2) {
    return {v1.m_x * v2.m_x, v1.m_y * v2.m_y, v1.m_z * v2.m_z};
}

template <typename T>
constexpr Vector3<T> operator*(Vector3Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x * val, v.m_y * val, v.m_z * val};
}

template <typename T>
constexpr Vector3<T> operator/(Vector3Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x / val, v.m_y / val, v.m_z / val};
}

template <typename T>
constexpr Vector3<T> operator+(Vector3Param<T> v, std::type_identity_t<T> val) {
    return {v.m_x + val, v.m_y + val, v.m_z + val};
}

template <typename T>
constexpr Vector3<T> operator+(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.m_x + val.m_x, v.m_y + val.m_y, v.m_z + val.m_z};
}

template <typename T>
constexpr Vector3<T> operator-(Vector3Param<T> v, Vector3Param<T> val) {
    return {v.m_x - val.m_x, v.m_y - val.m_y, v.m_z - val.m_z};
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
