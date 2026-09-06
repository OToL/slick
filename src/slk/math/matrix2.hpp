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
            ColumnType m_column0;
            ColumnType m_column1;
        };
        ColumnType m_columns[2];
        ScalarType m_values[4];
        struct {
            ScalarType m_00, m_10;
            ScalarType m_01, m_11;
        };
    };

    constexpr Matrix2() = default;

    constexpr Matrix2(ColumnParamType col0, ColumnParamType col1)
        : m_column0(col0)
        , m_column1(col1) { }

    constexpr Matrix2(ScalarType s00, ScalarType s01, ScalarType s10, ScalarType s11)
        : m_00(s00), m_10(s10)
        , m_01(s01), m_11(s11)
    {}

    constexpr Matrix2& setRotation(slk::f32 rot_rad) {
        auto const cos_val = std::cos(rot_rad);
        auto const sin_val = std::sin(rot_rad);

        m_column0 = {cos_val, sin_val};
        m_column1 = {-sin_val, cos_val};

        return *this;
    }

    constexpr Matrix2& setIdentity() {
        m_column0 = {1, 0};
        m_column1 = {0, 1};

        return *this;
    }

    constexpr ScalarType* data() {
        return &m_values[0];
    }

    constexpr ScalarType const* data() const {
        return &m_values[0];
    }

    constexpr ColumnType column(u32 idx) const {
        return m_columns[idx];
    }

    constexpr ColumnType column(u32 idx) {
        return m_columns[idx];
    }

    constexpr RowType row(u32 idx) const {
        return {m_columns[0][idx], m_columns[1][idx]};
    }

    constexpr RowType row(u32 idx) {
        return {m_columns[0][idx], m_columns[1][idx]};
    }

    constexpr ScalarType value(u32 row_idx, u32 col_idx) const {
        return m_columns[col_idx][row_idx];
    }

    constexpr Matrix2& set_value(u32 row_idx, u32 col_idx, ScalarType val) const {
        m_columns[col_idx][row_idx] = val;

        return *this;
    }

    constexpr ScalarType determinant() const {
       return m_00*m_11 - m_01*m_10; 
    }

    constexpr Matrix2& inverse() {
        const ScalarType inv_det = 1.f/determinant();

        const ScalarType tmp = m_00;
        m_00 = m_11 * inv_det;
        m_11 = tmp * inv_det;

        m_01 = -m_01 * inv_det;
        m_10 = -m_10 * inv_det;

        return *this;
    }

    // Transforms factory

    static constexpr Matrix2 makeRotation(slk::f32 rot_rad) {
        Matrix2 rot_matrix;
        rot_matrix.setRotation(rot_rad);

        return rot_matrix;
    }

    // Constants

    static constexpr Matrix2 indentity() {
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
    return {v.m_x * mat.m_column0.m_x + v.m_y * mat.m_column1.m_x, v.m_x * mat.m_column0.m_y + v.m_y * mat.m_column1.m_y};
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
