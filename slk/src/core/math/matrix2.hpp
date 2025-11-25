#pragma once

#include <core/core.hpp>
#include "math.hpp"
#include "vector2.hpp"

namespace slk {

template <typename T>
struct Matrix2 {
    using ParamType = Matrix2Param<T>;
    using RowType = slk::Vector2<T>;
    using RowParamType = typename RowType::ParamType;
    using ScalarType = typename slk::Vector2<T>::ScalarType;

    union {
        struct {
            RowType row0;
            RowType row1;
        };
        RowType rows[2];
        ScalarType values[4];
    };

    Matrix2() = default;

    Matrix2(RowParamType in_row0, RowParamType in_row1)
        : row0(in_row0)
        , row1(in_row1) { }

    Matrix2& set_rotation(slk::f32 rot_rad) {
        row0 = {std::cos(rot_rad), std::sin(rot_rad)};
        row1 = {-std::sin(rot_rad), std::cos(rot_rad)};

        return *this;
    }

    void set_identity() {
        row0 = {1, 0};
        row1 = {0, 1};
    }

    [[nodiscard]] ScalarType* data() {
        return &values[0];
    }

    [[nodiscard]] ScalarType const* data() const {
        return &values[0];
    }

    [[nodiscard]] RowType& operator [] (slk::u32 row_idx) {
        return rows[row_idx];
    }

    [[nodiscard]] RowType const& operator [] (slk::u32 row_idx) const {
        return rows[row_idx];
    }

    // Transforms factory

    [[nodiscard]] static Matrix2 make_rotation(slk::f32 rot_rad) {
        Matrix2 rot_matrix;
        rot_matrix.set_rotation(rot_rad);

        return rot_matrix;
    }

    // Constants

    [[nodiscard]] static inline constexpr Matrix2 indentity() {
        return {{1, 0}, {0, 1}};
    }

    static const Matrix2 IDENTITY;
};

// forward declaration required by Clang and MSVC
template <>
const Matrix2<slk::f32> Matrix2<slk::f32>::IDENTITY;
template <>
const Matrix2<slk::i32> Matrix2<slk::i32>::IDENTITY;

template <typename T>
[[nodiscard]] slk::Vector2<T> operator*(slk::Vector2<T> const& v, slk::Matrix2<T> const& mat) {
    return {v.x * mat.row0.x + v.y * mat.row1.x, v.x * mat.row0.y + v.y * mat.row1.y};
}

using Matrix2f = Matrix2<slk::f32>;
using Matrix2i = Matrix2<slk::i32>;

} // namespace slk
