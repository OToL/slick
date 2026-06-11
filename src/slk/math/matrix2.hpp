#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector2.hpp"

namespace slk {

template <typename T>
struct Matrix2 {

    using ParamType = Matrix2Param<T>;
    using RowType = slk::Vector2<T>;
    using ColumnType = slk::Vector2<T>;
    using ColumnParamType = typename ColumnType::ParamType;
    using RowParamType = typename RowType::ParamType;
    using ScalarType = typename slk::Vector2<T>::ScalarType;

    union {
        struct {
            ColumnType column0;
            ColumnType column1;
        };
        ColumnType columns[2];
        ScalarType values[4];
        struct {
            ScalarType m00, m10;
            ScalarType m01, m11;
        };
    };

    Matrix2() = default;

    Matrix2(ColumnParamType _col0, ColumnParamType _col1)
        : column0(_col0)
        , column1(_col1) { }

    Matrix2(ScalarType _m00, ScalarType _m01, ScalarType _m10, ScalarType _m11)
        : m00(_m00), m10(_m10)
        , m01(_m01), m11(_m11)
    {}

    void set_rotation(slk::f32 rot_rad) {
        auto const cos_val = std::cos(rot_rad);
        auto const sin_val = std::sin(rot_rad);

        column0 = {cos_val, sin_val};
        column1 = {-sin_val, cos_val};
    }

    void set_identity() {
        column0 = {1, 0};
        column1 = {0, 1};
    }

    ScalarType* data() {
        return &values[0];
    }

    ScalarType const* data() const {
        return &values[0];
    }

    ColumnType column(u32 idx) const {
        return columns[idx];
    }

    ColumnType column(u32 idx) {
        return columns[idx];
    }

    RowType row(u32 idx) const {
        return {columns[0][idx], columns[1][idx]};
    }

    RowType row(u32 idx) {
        return {columns[0][idx], columns[1][idx]};
    }

    ScalarType value(u32 row_idx, u32 col_idx) const {
        return columns[col_idx][row_idx];
    }

    void set_value(u32 row_idx, u32 col_idx, ScalarType val) const {
        columns[col_idx][row_idx] = val;
    }

    ScalarType determinant() const {
       return m00*m11 - m01*m10; 
    }

    void inverse() {
        const ScalarType inv_det = 1.f/determinant();

        const ScalarType tmp = m00;
        m00 = m11 * inv_det;
        m11 = tmp * inv_det;

        m01 = -m01 * inv_det;
        m10 = -m10 * inv_det;
    }

    // Transforms factory

    static Matrix2 make_rotation(slk::f32 rot_rad) {
        Matrix2 rot_matrix;
        rot_matrix.set_rotation(rot_rad);

        return rot_matrix;
    }

    // Constants

    static inline constexpr Matrix2 indentity() {
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
slk::Vector2<T> operator*(slk::Matrix2<T> const& mat, slk::Vector2<T> const& v) {
    return {v.x * mat.column0.x + v.y * mat.column1.x, v.x * mat.column0.y + v.y * mat.column1.y};
}
template <typename T>
slk::Matrix2<T> operator*(slk::Matrix2<T> const& lval, slk::Matrix2<T> const& rval) {
    return {
        lval * rval.column0,
        lval * rval.column1,
    };
}


using Matrix2f = Matrix2<slk::f32>;
using Matrix2i = Matrix2<slk::i32>;

} // namespace slk
