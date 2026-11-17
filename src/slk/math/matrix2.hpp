#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector2.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] Matrix2 {

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

    // Values are given column by column (same order as the memory layout)
    constexpr Matrix2(ScalarType s00, ScalarType s10,
                      ScalarType s01, ScalarType s11)
        : Matrix2(ColumnType{s00, s10}, ColumnType{s01, s11}) // keeps the columns as the active union member (constant evaluation)
    {}

    constexpr Matrix2& setRotation(ScalarType rot_rad) requires std::floating_point<T> {
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

    constexpr Matrix2& transpose() {
        const ScalarType tmp = m_10;
        m_10 = m_01;
        m_01 = tmp;

        return *this;
    }

    constexpr Matrix2 transposed() const {
        return {
            m_00, m_01,
            m_10, m_11,
        };
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

    constexpr RowType row(u32 idx) const {
        return {m_columns[0][idx], m_columns[1][idx]};
    }

    constexpr ScalarType value(u32 row_idx, u32 col_idx) const {
        return m_columns[col_idx][row_idx];
    }

    constexpr Matrix2& setValue(u32 row_idx, u32 col_idx, ScalarType val) {
        m_columns[col_idx][row_idx] = val;

        return *this;
    }

    constexpr ScalarType determinant() const {
       return m_00*m_11 - m_01*m_10;
    }

    constexpr Matrix2& inverse() requires std::floating_point<T> {
        const ScalarType det = determinant();
        assert(det != 0 && "inverting a singular matrix");
        const ScalarType inv_det = ScalarType(1)/det;

        const ScalarType tmp = m_00;
        m_00 = m_11 * inv_det;
        m_11 = tmp * inv_det;

        m_01 = -m_01 * inv_det;
        m_10 = -m_10 * inv_det;

        return *this;
    }

    constexpr Matrix2 inversed() const requires std::floating_point<T> {
        Matrix2 mat_inv = *this;
        mat_inv.inverse();

        return mat_inv;
    }

    // Transforms factory

    static constexpr Matrix2 makeRotation(ScalarType rot_rad) requires std::floating_point<T> {
        Matrix2 rot_matrix;
        rot_matrix.setRotation(rot_rad);

        return rot_matrix;
    }

    // Constants

    static consteval Matrix2 IDENTITY() {
        return {{1, 0}, {0, 1}};
    }
};

template <typename T>
constexpr slk::Vector2<T> operator*(slk::Matrix2<T> const& mat, slk::Vector2<T> const& v) {
    return {v.m_x * mat.m_column0.m_x + v.m_y * mat.m_column1.m_x, v.m_x * mat.m_column0.m_y + v.m_y * mat.m_column1.m_y};
}
template <typename T>
constexpr slk::Matrix2<T> operator*(slk::Matrix2<T> const& lval, slk::Matrix2<T> const& rval) {
    return {
        lval * rval.m_column0,
        lval * rval.m_column1,
    };
}

using Matrix2f = Matrix2<slk::f32>;
using Matrix2i = Matrix2<slk::i32>;

} // namespace slk
