#pragma once

#include <cmath>

namespace slk {

template <typename T>
struct Vector2
{
    using ScalarType = T;

    union
    {
        struct
        {
            ScalarType x;
            ScalarType y;
        };
        ScalarType data[2];
    };

    static constexpr Vector2 zero()
    {
        return {0, 0};
    }
    static constexpr Vector2 unitY()
    {
        return {0, 1};
    };
    static constexpr Vector2 unitX()
    {
        return {1, 0};
    };

    Vector2() = default;

    explicit Vector2(ScalarType val)
        : x(val)
        , y(val)
    {
    }

    Vector2(ScalarType in_x, ScalarType in_y)
        : x(in_x)
        , y(in_y)
    {
    }

    Vector2 & operator*=(ScalarType val)
    {
        x *= val;
        y *= val;

        return *this;
    }

    Vector2 & operator-=(Vector2 const & v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2 & operator+=(Vector2 const & v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2 operator-() const
    {
        return {-x, -y};
    }

    ScalarType dot(Vector2 v) const
    {
        return x * v.x + y * v.y;
    }

    ScalarType length() const
    {
        return std::sqrt(x * x + y * y);
    }

    ScalarType length2() const
    {
        return x * x + y * y;
    }

    Vector2 & normalize()
    {
        auto const length_val = length();
        x /= length_val;
        y /= length_val;

        return *this;
    }

    Vector2 normalized() const
    {
        auto const length_val = length();

        return {
            x /= length_val,
            y /= length_val,
        };
    }

};

using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;

template <typename T>
inline Vector2<T> operator*(Vector2<T> v1, Vector2<T> v2)
{
    return {v1.x * v2.x, v1.y * v2.y};
}

template <typename T>
inline Vector2<T> operator*(Vector2<T> v, T val)
{
    return {v.x * val, v.y * val};
}

template <typename T>
inline Vector2<T> operator/(Vector2<T> v, T val)
{
    return {v.x / val, v.y / val};
}

template <typename T>
inline Vector2<T> operator+(Vector2<T> v, T val)
{
    return {v.x + val, v.y + val};
}

template <typename T>
inline Vector2<T> operator+(Vector2<T> v, Vector2<T> val)
{
    return {v.x + val.x, v.y + val.y};
}

template <typename T>
inline Vector2<T> operator-(Vector2<T> v, Vector2<T> val)
{
    return {v.x - val.x, v.y - val.y};
}

template <typename T>
bool operator<(slk::Vector2<T> const & v1, slk::Vector2<T> const & v2)
{
    return v1.x < v2.x && v1.y < v2.y;
}

template <typename T>
bool operator>(slk::Vector2<T> const & v1, slk::Vector2<T> const & v2)
{
    return v1.x > v2.x && v1.y > v2.y;
}

template <typename T>
constexpr slk::Vector2<T> minComponents(slk::Vector2<T> lval, slk::Vector2<T> rval) {
    return {
        lval.x < rval.x ? lval.x : rval.x,
        lval.y < rval.y ? lval.y : rval.y,
    };
}

template <typename T>
constexpr slk::Vector2<T> maxComponents(slk::Vector2<T> lval, slk::Vector2<T> rval) {
    return {
        lval.x > rval.x ? lval.x : rval.x,
        lval.y > rval.y ? lval.y : rval.y,
    };
}

} // namespace slk
