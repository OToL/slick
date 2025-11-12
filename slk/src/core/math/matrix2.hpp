#pragma once

#include <core/core.hpp>
#include "vector2.hpp"

namespace slk {

template <typename T>
struct Matrix2
{
    using RowType = slk::Vector2<T>;
    using ScalarType = typename slk::Vector2<T>::ScalarType;

    RowType rows[2];

    // TODO: move to a 'transform' stuct
    static Matrix2 makeRotation(slk::f32 rot_rad)
    {
        return {
            {
                Vector2<T>{std::cos(rot_rad), -std::sin(rot_rad)}, 
                Vector2<T>{std::sin(rot_rad), std::cos(rot_rad)}
            }
        };
    }

    // TODO: move to a 'transform' stuct
    static inline constexpr Matrix2 indentity() {
        return {
            {1,0},
            {0,1}
        };
    }
};

template <typename T>
slk::Vector2<T> operator * (slk::Matrix2<T> const& mat, slk::Vector2<T> const& v) {

    return {
        v.dot(mat.rows[0]),
        v.dot(mat.rows[1])
    };
}


using Matrix2f = Matrix2<slk::f32>;
using Matrix2i = Matrix2<slk::i32>;

} // namespace slk
