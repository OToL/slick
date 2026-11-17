#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector3.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] Matrix3 {
    using ParamType = Matrix3Param<T>;
    using RowType = slk::Vector3<T>;
    using ColumnType = slk::Vector3<T>;
    using RowParamType = typename RowType::ParamType;
    using ColumnParamType = typename ColumnType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            ColumnType m_column0;
            ColumnType m_column1;
            ColumnType m_column2;
        };
        ColumnType m_columns[3];
        ScalarType m_values[9];
        struct {
            ScalarType m_00, m_10, m_20;
            ScalarType m_01, m_11, m_21;
            ScalarType m_02, m_12, m_22;
        };
    };

    constexpr Matrix3() = default;

    constexpr Matrix3(ColumnParamType col0, ColumnParamType col1, ColumnParamType col2)
        : m_column0(col0)
        , m_column1(col1)
        , m_column2(col2) { }

    // Values are given column by column (same order as the memory layout)
    constexpr Matrix3(ScalarType s00, ScalarType s10, ScalarType s20,
            ScalarType s01, ScalarType s11, ScalarType s21,
            ScalarType s02, ScalarType s12, ScalarType s22)
        : Matrix3(ColumnType{s00, s10, s20}, ColumnType{s01, s11, s21}, ColumnType{s02, s12, s22}) // keeps the columns as the active union member (constant evaluation)
    {}

    constexpr Matrix3& transpose()
    {
        for (u32 col_idx = 0 ; col_idx != 3 ; ++col_idx) {
            for (u32 row_idx = col_idx + 1; row_idx != 3; ++row_idx)
            {
                const ScalarType tmp = m_columns[col_idx][row_idx];
                m_columns[col_idx][row_idx] = m_columns[row_idx][col_idx];
                m_columns[row_idx][col_idx] = tmp;
            }
        }

        return *this;
    }

    constexpr Matrix3 transposed() const {
        return {
            m_values[0], m_values[3], m_values[6],
            m_values[1], m_values[4], m_values[7],
            m_values[2], m_values[5], m_values[8],
        };
    }

    constexpr Matrix3& setIdentity() {
        m_column0 = {1, 0, 0};
        m_column1 = {0, 1, 0};
        m_column2 = {0, 0, 1};

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

    constexpr RowType row(u32 idx) const {
        return {m_columns[0][idx], m_columns[1][idx], m_columns[2][idx]};
    }

    constexpr ScalarType value(u32 row_idx, u32 col_idx) const {
        return m_columns[col_idx][row_idx];
    }

    constexpr ColumnType xAxis() const {
        return m_column0;
    }

    constexpr ColumnType yAxis() const {
        return m_column1;
    }

    constexpr ColumnType zAxis() const {
        return m_column2;
    }

    constexpr Matrix3& setValue(u32 row_idx, u32 col_idx, ScalarType val) {
        m_columns[col_idx][row_idx] = val;

        return *this;
    }

    // Sign matrix used for cofactor matrix and determinant
    //
    //  [+ - +]
    //  [- + -]
    //  [+ - +]
    //
    constexpr ScalarType determinant() const {
       return m_00*(m_11*m_22 - m_21*m_12) - m_10*(m_01*m_22 - m_21*m_02) + m_20*(m_01*m_12 - m_11*m_02); 
    }

    constexpr Matrix3 inversed() const requires std::floating_point<T> {
        Matrix3 mat_inv = *this;
        mat_inv.inverse();

        return mat_inv;
    }

    constexpr Matrix3& inverse() requires std::floating_point<T> {

		const ScalarType _m00 = m_values[0];
		const ScalarType _m10 = m_values[1];
		const ScalarType _m20 = m_values[2];
		const ScalarType _m01 = m_values[3];
		const ScalarType _m11 = m_values[4];
		const ScalarType _m21 = m_values[5];
		const ScalarType _m02 = m_values[6];
		const ScalarType _m12 = m_values[7];
		const ScalarType _m22 = m_values[8];

		const ScalarType det = determinant();
		assert(det != 0 && "inverting a singular matrix");
		const ScalarType invDet = ScalarType(1)/det;

		m_values[0] = +(_m11*_m22 - _m21*_m12) * invDet;
		m_values[1] = -(_m10*_m22 - _m20*_m12) * invDet;
		m_values[2] = +(_m10*_m21 - _m20*_m11) * invDet;

		m_values[3] = -(_m01*_m22 - _m21*_m02) * invDet;
		m_values[4] = +(_m00*_m22 - _m20*_m02) * invDet;
		m_values[5] = -(_m00*_m21 - _m20*_m01) * invDet;

		m_values[6] = +(_m01*_m12 - _m11*_m02) * invDet;
		m_values[7] = -(_m00*_m12 - _m10*_m02) * invDet;
		m_values[8] = +(_m00*_m11 - _m10*_m01) * invDet;

        return *this;
    }

    constexpr Matrix3& setRotationX(ScalarType rot_rad) requires std::floating_point<T>
	{
		const auto sx = std::sin(rot_rad);
		const auto cx = std::cos(rot_rad);

		m_values[ 0] = 1.0f;
		m_values[ 1] = 0.f;
		m_values[ 2] = 0.f;

		m_values[ 3] = 0.f;
		m_values[ 4] = cx;
		m_values[ 5] = sx;

		m_values[ 6] = 0.f;
		m_values[ 7] = -sx;
		m_values[ 8] = cx;

        return *this;
	}

	constexpr Matrix3& setRotationY(ScalarType rot_rad) requires std::floating_point<T>
	{
		const ScalarType sy = std::sin(rot_rad);
		const ScalarType cy = std::cos(rot_rad);

		m_values[0] = cy;
		m_values[1] = 0.f;
		m_values[2] = -sy;

		m_values[3] = 0.f;
		m_values[4] = 1.0f;
		m_values[5] = 0.f;

		m_values[6] = sy;
		m_values[7] = 0.f;
		m_values[8] = cy;

        return *this;
	}

	constexpr Matrix3& setRotationZ(ScalarType rot_rad) requires std::floating_point<T>
	{
		const ScalarType sz = std::sin(rot_rad);
		const ScalarType cz = std::cos(rot_rad);

		m_values[0] = cz;
		m_values[1] = sz;
		m_values[2] = 0.f;

		m_values[3] = -sz;
		m_values[4] = cz;
		m_values[5] = 0.f;

		m_values[6] = 0.f;
		m_values[7] = 0.f;
		m_values[8] = 1.0f;

        return *this;
	}

	constexpr Matrix3& setRotationXY(ScalarType rotx_rad, ScalarType roty_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);

		m_values[0] = cy;
		m_values[1] = sx*sy;
		m_values[2] = -cx*sy;

		m_values[3] = 0.f;
		m_values[4] = cx;
		m_values[5] = sx;

		m_values[6] = sy;
		m_values[7] = -sx*cy;
		m_values[8] = cx*cy;

        return *this;
	}

	constexpr Matrix3& setRotationXYZ(ScalarType rotx_rad, ScalarType roty_rad, ScalarType rotz_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);
		const ScalarType sz = std::sin(rotz_rad);
		const ScalarType cz = std::cos(rotz_rad);

		m_values[0] = cy*cz;
		m_values[1] = cz*sx*sy + cx*sz;
		m_values[2] = -cx*cz*sy + sx*sz;

		m_values[3] = -cy*sz;
		m_values[4] = cx*cz - sx*sy*sz;
		m_values[5] = cz*sx + cx*sy*sz;

		m_values[6] = sy;
		m_values[7] = -cy*sx;
		m_values[8] = cx*cy;

        return *this;
	}

	constexpr Matrix3& setRotationZYX(ScalarType rotx_rad, ScalarType roty_rad, ScalarType rotz_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);
		const ScalarType sz = std::sin(rotz_rad);
		const ScalarType cz = std::cos(rotz_rad);

		m_values[0] = cy*cz;
		m_values[1] = cy*sz;
		m_values[2] = -sy;

		m_values[3] = cz*sx*sy-cx*sz;
		m_values[4] = cx*cz + sx*sy*sz;
		m_values[5] = cy*sx;

		m_values[6] = cx*cz*sy+sx*sz;
		m_values[7] = -cz*sx + cx*sy*sz;
		m_values[8] = cx*cy;

        return *this;
	};

    // Constants

    static consteval Matrix3 IDENTITY() {
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }
};

template <typename T>
constexpr slk::Vector3<T> operator*(slk::Matrix3<T> const& mat, slk::Vector3<T> const& v) {
    return {
        v.m_x * mat.m_column0.m_x + v.m_y * mat.m_column1.m_x + v.m_z * mat.m_column2.m_x, 
        v.m_x * mat.m_column0.m_y + v.m_y * mat.m_column1.m_y + v.m_z * mat.m_column2.m_y,
        v.m_x * mat.m_column0.m_z + v.m_y * mat.m_column1.m_z + v.m_z * mat.m_column2.m_z,
    };
}

template <typename T>
constexpr slk::Matrix3<T> operator*(slk::Matrix3<T> const& lval, slk::Matrix3<T> const& rval) {
    return {
        lval * rval.m_column0,
        lval * rval.m_column1,
        lval * rval.m_column2,
    };
}

using Matrix3f = Matrix3<slk::f32>;
using Matrix3i = Matrix3<slk::i32>;

} // namespace slk
