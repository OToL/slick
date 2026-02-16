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
            ScalarType x;
            ScalarType y;
        };
        ScalarType values[2];
    };

    Vector2() = default;

    explicit Vector2(ScalarType val)
        : x(val)
        , y(val) { }

    Vector2(ScalarType _x, ScalarType _y)
        : x(_x)
        , y(_y) { }

    ScalarType dot(ParamType v) const {
        return x * v.x + y * v.y;
    }

    Vector2& mul(ScalarType v) {
        x *= v;
        y *= v;

        return *this;
    }

    Vector2 multiplied(ScalarType v) const {
        return {x * v, y * v};
    }

    Vector2& mul(ParamType v) {
        x *= v.x;
        y *= v.y;

        return *this;
    }

    Vector2 multiplied(ParamType v) const {
        return {x * v.x, y * v.y};
    }

    Vector2& normalize() {
        auto const length_val = length();
        x /= length_val;
        y /= length_val;

        return *this;
    }

    Vector2 normalized() const {
        auto const length_val = length();
        return {
            x /= length_val,
            y /= length_val,
        };
    }

    ScalarType length() const {
        return std::sqrt(x * x + y * y);
    }

    ScalarType length2() const {
        return x * x + y * y;
    }

    ScalarType* data() {
        return &values[0];
    }

    ScalarType const* data() const {
        return &values[0];
    }

    // Operators

    Vector2& operator*=(ScalarType val) {
        x *= val;
        y *= val;

        return *this;
    }

    Vector2& operator-=(ParamType const& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2& operator+=(ParamType const& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2 operator-() const {
        return {-x, -y};
    }

    ScalarType operator[](slk::u32 idx) const {
        // TODO: add assertion on idx value
        return values[idx];
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
const Vector2<slk::f32> Vector2<slk::f32>::ZERO;
template <>
const Vector2<slk::f32> Vector2<slk::f32>::UNITX;
template <>
const Vector2<slk::f32> Vector2<slk::f32>::UNITY;
template <>
const Vector2<slk::i32> Vector2<slk::i32>::ZERO;
template <>
const Vector2<slk::i32> Vector2<slk::i32>::UNITX;
template <>
const Vector2<slk::i32> Vector2<slk::i32>::UNITY;

template <typename T>
inline Vector2<T> operator*(Vector2Param<T> v1, Vector2Param<T> v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

template <typename T>
inline Vector2<T> operator*(Vector2Param<T> v, T val) {
    return {v.x * val, v.y * val};
}

template <typename T>
inline Vector2<T> operator/(Vector2Param<T> v, T val) {
    return {v.x / val, v.y / val};
}

template <typename T>
inline Vector2<T> operator+(Vector2Param<T> v, T val) {
    return {v.x + val, v.y + val};
}

template <typename T>
inline Vector2<T> operator+(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.x + val.x, v.y + val.y};
}

template <typename T>
inline Vector2<T> operator-(Vector2Param<T> v, Vector2Param<T> val) {
    return {v.x - val.x, v.y - val.y};
}

template <typename T>
bool operator<(slk::Vector2Param<T> const& v1, slk::Vector2Param<T> const& v2) {
    return v1.x < v2.x && v1.y < v2.y;
}

template <typename T>
bool operator>(slk::Vector2Param<T> const& v1, slk::Vector2Param<T> const& v2) {
    return v1.x > v2.x && v1.y > v2.y;
}

// component wise min/max
template <typename T>
constexpr slk::Vector2<T> min(slk::Vector2Param<T> lval, slk::Vector2Param<T> rval) {
    return {
        lval.x < rval.x ? lval.x : rval.x,
        lval.y < rval.y ? lval.y : rval.y,
    };
}

template <typename T>
constexpr slk::Vector2<T> max(slk::Vector2Param<T> lval, slk::Vector2Param<T> rval) {
    return {
        lval.x > rval.x ? lval.x : rval.x,
        lval.y > rval.y ? lval.y : rval.y,
    };
}

using Vector2i = Vector2<slk::i32>;
using Vector2f = Vector2<slk::f32>;

} // namespace slk
