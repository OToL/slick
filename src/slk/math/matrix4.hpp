#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector4.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"

#include <assert.h>

import std;

namespace slk {

template <typename T>
struct [[nodiscard]] Matrix4 {
    using ParamType = Matrix4Param<T>;
    using RowType = slk::Vector4<T>;
    using ColumnType = slk::Vector4<T>;
    using RowParamType = typename RowType::ParamType;
    using ColumnParamType = typename ColumnType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            ColumnType m_column0;
            ColumnType m_column1;
            ColumnType m_column2;
            ColumnType m_column3;
        };
        ColumnType m_columns[4];
        ScalarType m_values[16];
        struct {
            ScalarType m_00, m_10, m_20, m_30;
            ScalarType m_01, m_11, m_21, m_31;
            ScalarType m_02, m_12, m_22, m_32;
            ScalarType m_03, m_13, m_23, m_33;
        };
    };

    constexpr Matrix4() = default;

    constexpr Matrix4(ColumnParamType col0, ColumnParamType col1, ColumnParamType col2, ColumnParamType col3)
        : m_column0(col0)
        , m_column1(col1)
        , m_column2(col2)
        , m_column3(col3) { }

    // Values are given column by column (same order as the memory layout)
    constexpr Matrix4(ScalarType s00, ScalarType s10, ScalarType s20, ScalarType s30,
            ScalarType s01, ScalarType s11, ScalarType s21, ScalarType s31,
            ScalarType s02, ScalarType s12, ScalarType s22, ScalarType s32,
            ScalarType s03, ScalarType s13, ScalarType s23, ScalarType s33)
        : Matrix4(ColumnType{s00, s10, s20, s30}, ColumnType{s01, s11, s21, s31}, ColumnType{s02, s12, s22, s32},
                  ColumnType{s03, s13, s23, s33}) // keeps the columns as the active union member (constant evaluation)
    {}

    constexpr Matrix4& setIdentity() {
        m_column0 = {1, 0, 0, 0};
        m_column1 = {0, 1, 0, 0};
        m_column2 = {0, 0, 1, 0};
        m_column3 = {0, 0, 0, 1};

        return *this;
    }

    constexpr Matrix4& transpose()
    {
        for (u32 col_idx = 0 ; col_idx != 4 ; ++col_idx) {
            for (u32 row_idx = col_idx + 1; row_idx != 4; ++row_idx)
            {
                const ScalarType tmp = m_columns[col_idx][row_idx];
                m_columns[col_idx][row_idx] = m_columns[row_idx][col_idx];
                m_columns[row_idx][col_idx] = tmp;
            }
        }

        return *this;
    }

    constexpr Matrix4 transposed() const {
        return {
            m_values[0], m_values[4], m_values[ 8], m_values[12],
            m_values[1], m_values[5], m_values[ 9], m_values[13],
            m_values[2], m_values[6], m_values[10], m_values[14],
            m_values[3], m_values[7], m_values[11], m_values[15],
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
        return {m_columns[0][idx], m_columns[1][idx], m_columns[2][idx], m_columns[3][idx]};
    }

    constexpr ColumnType translation() const {
        return m_column3;
    }

    constexpr Matrix4& setTranslation(ColumnParamType translation) {
        m_column3 = translation;

        return *this;
    }

    constexpr Matrix4& setTranslation(Vector3<ScalarType> translation) {
        m_column3 = {translation.m_x, translation.m_y, translation.m_z, 1.f};

        return *this;
    }

    constexpr slk::Matrix3<ScalarType> rotation() const {
        return {
            m_00, m_10, m_20,
            m_01, m_11, m_21,
            m_02, m_12, m_22
        };
    }

    constexpr Matrix4& setRotation(Matrix3<ScalarType>::ParamType rot) requires std::floating_point<T> {
        m_column0 = {rot.m_column0.m_x, rot.m_column0.m_y, rot.m_column0.m_z, 0.f};
        m_column1 = {rot.m_column1.m_x, rot.m_column1.m_y, rot.m_column1.m_z, 0.f};
        m_column2 = {rot.m_column2.m_x, rot.m_column2.m_y, rot.m_column2.m_z, 0.f};

        return *this;
    }

    constexpr ScalarType value(u32 row_idx, u32 col_idx) const {
        return m_columns[col_idx][row_idx];
    }

    constexpr Matrix4& setValue(u32 row_idx, u32 col_idx, ScalarType val) {
        m_columns[col_idx][row_idx] = val;

        return *this;
    }

    constexpr Vector3<ScalarType> xAxis() const {
        return {m_column0.m_x, m_column0.m_y, m_column0.m_z};
    }

    constexpr Vector3<ScalarType> yAxis() const {
        return {m_column1.m_x, m_column1.m_y, m_column1.m_z};
    }

    constexpr Vector3<ScalarType> zAxis() const {
        return {m_column2.m_x, m_column2.m_y, m_column2.m_z};
    }

    // Sign matrix used for cofactor matrix and determinant
    //
    //  [+ - + -]
    //  [- + - +]
    //  [+ - + -]
    //  [- + - +]
    //
    constexpr ScalarType determinant() const {
        return m_00 * (m_11*(m_22*m_33 - m_32*m_23) - m_21*(m_12*m_33 - m_32*m_13) + m_31*(m_12*m_23 - m_22*m_13) )
            - m_10 * (m_01*(m_22*m_33 - m_32*m_23) - m_21*(m_02*m_33 - m_32*m_03) + m_31*(m_02*m_23 - m_22*m_03) )
            + m_20 * (m_01*(m_12*m_33 - m_32*m_13) - m_11*(m_02*m_33 - m_32*m_03) + m_31*(m_02*m_13 - m_12*m_03) )
            - m_30 * (m_01*(m_12*m_23 - m_22*m_13) - m_11*(m_02*m_23 - m_22*m_03) + m_21*(m_02*m_13 - m_12*m_03) );
    }

    constexpr Matrix4& inverse() requires std::floating_point<T> {
		const ScalarType xx = m_values[ 0];
		const ScalarType xy = m_values[ 1];
		const ScalarType xz = m_values[ 2];
		const ScalarType xw = m_values[ 3];
		const ScalarType yx = m_values[ 4];
		const ScalarType yy = m_values[ 5];
		const ScalarType yz = m_values[ 6];
		const ScalarType yw = m_values[ 7];
		const ScalarType zx = m_values[ 8];
		const ScalarType zy = m_values[ 9];
		const ScalarType zz = m_values[10];
		const ScalarType zw = m_values[11];
		const ScalarType wx = m_values[12];
		const ScalarType wy = m_values[13];
		const ScalarType wz = m_values[14];
		const ScalarType ww = m_values[15];

		ScalarType det = 0;
		det += xx * (yy*(zz*ww - zw*wz) - yz*(zy*ww - zw*wy) + yw*(zy*wz - zz*wy) );
		det -= xy * (yx*(zz*ww - zw*wz) - yz*(zx*ww - zw*wx) + yw*(zx*wz - zz*wx) );
		det += xz * (yx*(zy*ww - zw*wy) - yy*(zx*ww - zw*wx) + yw*(zx*wy - zy*wx) );
		det -= xw * (yx*(zy*wz - zz*wy) - yy*(zx*wz - zz*wx) + yz*(zx*wy - zy*wx) );

		assert(det != 0 && "inverting a singular matrix");
		ScalarType inv_det = ScalarType(1)/det;

		m_values[ 0] = +(yy*(zz*ww - wz*zw) - yz*(zy*ww - wy*zw) + yw*(zy*wz - wy*zz) ) * inv_det;
		m_values[ 1] = -(xy*(zz*ww - wz*zw) - xz*(zy*ww - wy*zw) + xw*(zy*wz - wy*zz) ) * inv_det;
		m_values[ 2] = +(xy*(yz*ww - wz*yw) - xz*(yy*ww - wy*yw) + xw*(yy*wz - wy*yz) ) * inv_det;
		m_values[ 3] = -(xy*(yz*zw - zz*yw) - xz*(yy*zw - zy*yw) + xw*(yy*zz - zy*yz) ) * inv_det;

		m_values[ 4] = -(yx*(zz*ww - wz*zw) - yz*(zx*ww - wx*zw) + yw*(zx*wz - wx*zz) ) * inv_det;
		m_values[ 5] = +(xx*(zz*ww - wz*zw) - xz*(zx*ww - wx*zw) + xw*(zx*wz - wx*zz) ) * inv_det;
		m_values[ 6] = -(xx*(yz*ww - wz*yw) - xz*(yx*ww - wx*yw) + xw*(yx*wz - wx*yz) ) * inv_det;
		m_values[ 7] = +(xx*(yz*zw - zz*yw) - xz*(yx*zw - zx*yw) + xw*(yx*zz - zx*yz) ) * inv_det;

		m_values[ 8] = +(yx*(zy*ww - wy*zw) - yy*(zx*ww - wx*zw) + yw*(zx*wy - wx*zy) ) * inv_det;
		m_values[ 9] = -(xx*(zy*ww - wy*zw) - xy*(zx*ww - wx*zw) + xw*(zx*wy - wx*zy) ) * inv_det;
		m_values[10] = +(xx*(yy*ww - wy*yw) - xy*(yx*ww - wx*yw) + xw*(yx*wy - wx*yy) ) * inv_det;
		m_values[11] = -(xx*(yy*zw - zy*yw) - xy*(yx*zw - zx*yw) + xw*(yx*zy - zx*yy) ) * inv_det;

		m_values[12] = -(yx*(zy*wz - wy*zz) - yy*(zx*wz - wx*zz) + yz*(zx*wy - wx*zy) ) * inv_det;
		m_values[13] = +(xx*(zy*wz - wy*zz) - xy*(zx*wz - wx*zz) + xz*(zx*wy - wx*zy) ) * inv_det;
		m_values[14] = -(xx*(yy*wz - wy*yz) - xy*(yx*wz - wx*yz) + xz*(yx*wy - wx*yy) ) * inv_det;
		m_values[15] = +(xx*(yy*zz - zy*yz) - xy*(yx*zz - zx*yz) + xz*(yx*zy - zx*yy) ) * inv_det;

        return *this;
    }

    constexpr Matrix4 inversed() const requires std::floating_point<T> {
        Matrix4 mat_inv = *this;
        mat_inv.inverse();

        return mat_inv;
    }

	constexpr Matrix4& setRotationX(ScalarType rot_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rot_rad);
		const ScalarType cx = std::cos(rot_rad);

		m_values[ 0] = 1.0f;
		m_values[ 1] = 0.f;
		m_values[ 2] = 0.f;
		m_values[ 3] = 0.f;

		m_values[ 4] = 0.f;
		m_values[ 5] = cx;
		m_values[ 6] = sx;
		m_values[ 7] = 0.f;

		m_values[ 8] = 0.f;
		m_values[ 9] = -sx;
		m_values[10] = cx;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	}

	constexpr Matrix4& setRotationY(ScalarType rot_rad) requires std::floating_point<T>
	{
		const ScalarType sy = std::sin(rot_rad);
		const ScalarType cy = std::cos(rot_rad);

		m_values[ 0] = cy;
		m_values[ 1] = 0.f;
		m_values[ 2] = -sy;
		m_values[ 3] = 0.f;

		m_values[ 4] = 0.f;
		m_values[ 5] = 1.0f;
		m_values[ 6] = 0.f;
		m_values[ 7] = 0.f;

		m_values[ 8] = sy;
		m_values[ 9] = 0.f;
		m_values[10] = cy;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	}

	constexpr Matrix4& setRotationZ(ScalarType rot_rad) requires std::floating_point<T>
	{
		const ScalarType sz = std::sin(rot_rad);
		const ScalarType cz = std::cos(rot_rad);

		m_values[ 0] = cz;
		m_values[ 1] = sz;
		m_values[ 2] = 0.f;
		m_values[ 3] = 0.f;

		m_values[ 4] = -sz;
		m_values[ 5] = cz;
		m_values[ 6] = 0.f;
		m_values[ 7] = 0.f;

		m_values[ 8] = 0.f;
		m_values[ 9] = 0.f;
		m_values[10] = 1.0f;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	}

	constexpr Matrix4& setRotationXY(ScalarType rotx_rad, ScalarType roty_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);

		m_values[ 0] = cy;
		m_values[ 1] = sx*sy;
		m_values[ 2] = -cx*sy;
		m_values[ 3] = 0.f;

		m_values[ 4] = 0.f;
		m_values[ 5] = cx;
		m_values[ 6] = sx;
		m_values[ 7] = 0.f;

		m_values[ 8] = sy;
		m_values[ 9] = -sx*cy;
		m_values[10] = cx*cy;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	}

	constexpr Matrix4& setRotationXYZ(ScalarType rotx_rad, ScalarType roty_rad, ScalarType rotz_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);
		const ScalarType sz = std::sin(rotz_rad);
		const ScalarType cz = std::cos(rotz_rad);

		m_values[ 0] = cy*cz;
		m_values[ 1] = cz*sx*sy + cx*sz;
		m_values[ 2] = -cx*cz*sy + sx*sz;
		m_values[ 3] = 0.f;

		m_values[ 4] = -cy*sz;
		m_values[ 5] = cx*cz - sx*sy*sz;
		m_values[ 6] = cz*sx + cx*sy*sz;
		m_values[ 7] = 0.f;

		m_values[ 8] = sy;
		m_values[ 9] = -cy*sx;
		m_values[10] = cx*cy;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	}

	constexpr Matrix4& setRotationZYX(ScalarType rotx_rad, ScalarType roty_rad, ScalarType rotz_rad) requires std::floating_point<T>
	{
		const ScalarType sx = std::sin(rotx_rad);
		const ScalarType cx = std::cos(rotx_rad);
		const ScalarType sy = std::sin(roty_rad);
		const ScalarType cy = std::cos(roty_rad);
		const ScalarType sz = std::sin(rotz_rad);
		const ScalarType cz = std::cos(rotz_rad);

		m_values[ 0] = cy*cz;
		m_values[ 1] = cy*sz;
		m_values[ 2] = -sy;
		m_values[ 3] = 0.f;

		m_values[ 4] = cz*sx*sy-cx*sz;
		m_values[ 5] = cx*cz + sx*sy*sz;
		m_values[ 6] = cy*sx;
		m_values[ 7] = 0.f;

		m_values[ 8] = cx*cz*sy+sx*sz;
		m_values[ 9] = -cz*sx + cx*sy*sz;
		m_values[10] = cx*cy;
		m_values[11] = 0.f;

		m_values[12] = 0.f;
		m_values[13] = 0.f;
		m_values[14] = 0.f;
		m_values[15] = 1.0f;

        return *this;
	};

    // Constants

    static consteval Matrix4 IDENTITY() {
        return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    }
};

template <typename T>
constexpr slk::Vector4<T> operator*(slk::Matrix4<T> const& mat, slk::Vector4<T> const& v) {
    return {
        v.m_x * mat.m_column0.m_x + v.m_y * mat.m_column1.m_x + v.m_z * mat.m_column2.m_x + v.m_w * mat.m_column3.m_x, 
        v.m_x * mat.m_column0.m_y + v.m_y * mat.m_column1.m_y + v.m_z * mat.m_column2.m_y + v.m_w * mat.m_column3.m_y,
        v.m_x * mat.m_column0.m_z + v.m_y * mat.m_column1.m_z + v.m_z * mat.m_column2.m_z + v.m_w * mat.m_column3.m_z,
        v.m_x * mat.m_column0.m_w + v.m_y * mat.m_column1.m_w + v.m_z * mat.m_column2.m_w + v.m_w * mat.m_column3.m_w,
    };
}

template <typename T>
constexpr slk::Matrix4<T> operator*(slk::Matrix4<T> const& lval, slk::Matrix4<T> const& rval) {
    return {
        lval * rval.m_column0,
        lval * rval.m_column1,
        lval * rval.m_column2,
        lval * rval.m_column3
    };
}

using Matrix4f = Matrix4<slk::f32>;
using Matrix4i = Matrix4<slk::i32>;

} // namespace slk
