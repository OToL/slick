#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector3.hpp"

namespace slk {

template <typename T>
struct Matrix3 {
    using ParamType = Matrix3Param<T>;
    using RowType = slk::Vector3<T>;
    using ColumnType = slk::Vector3<T>;
    using RowParamType = typename RowType::ParamType;
    using ColumnParamType = typename ColumnType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            ColumnType column0;
            ColumnType column1;
            ColumnType column2;
        };
        ColumnType columns[3];
        ScalarType values[9];
        struct {
            ScalarType m00, m10, m20;
            ScalarType m01, m11, m21;
            ScalarType m02, m12, m22;
        };
    };

    Matrix3() = default;

    Matrix3(RowParamType _col0, RowParamType _col1, RowParamType _col2)
        : column0(_col0)
        , column1(_col1)
        , column2(_col2) { }

    Matrix3(ScalarType _m00, ScalarType _m01, ScalarType _m02,
            ScalarType _m10, ScalarType _m11, ScalarType _m12,
            ScalarType _m20, ScalarType _m21, ScalarType _m22)
        : m00(_m00), m10(_m10), m20(_m20)
        , m01(_m01), m11(_m11), m21(_m21)
        , m02(_m02), m12(_m12) , m22(_m22)
    {}

    void transpose()
    {
        for (u32 col_idx = 0 ; col_idx != 3 ; ++col_idx) {
            for (u32 row_idx = col_idx + 1; row_idx != 3; ++row_idx)
            {
                const ScalarType tmp = columns[col_idx][row_idx];
                columns[col_idx][row_idx] = columns[row_idx][col_idx];
                columns[row_idx][col_idx] = tmp;
            }
        }
    }

    Matrix3 transposed() const {
        return {
            values[0], values[3], values[6],
            values[1], values[4], values[7],
            values[2], values[5], values[8],
        };
    }

    void set_identity() {
        column0 = {1, 0, 0};
        column1 = {0, 1, 0};
        column2 = {0, 0, 1};
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
        return {columns[0][idx], columns[1][idx], columns[2][idx]};
    }

    RowType row(u32 idx) {
        return {columns[0][idx], columns[1][idx], columns[2][idx]};
    }

    ScalarType value(u32 row_idx, u32 col_idx) const {
        return columns[col_idx][row_idx];
    }

    ColumnType x_axis() const {
        return column0;
    }

    ColumnType y_axis() const {
        return column1;
    }

    ColumnType z_axis() const {
        return column2;
    }

    void set_value(u32 row_idx, u32 col_idx, ScalarType val) const {
        columns[col_idx][row_idx] = val;
    }

    // Sign matrix used for cofactor matrix and determinant
    //
    //  [+ - +]
    //  [- + -]
    //  [+ - +]
    //
    ScalarType determinant() const {
       return m00*(m11*m22 - m21*m12) - m10*(m01*m22 - m21*m02) + m20*(m01*m12 - m11*m02); 
    }

    Matrix3 inversed() const {
        Matrix3 mat_inv = *this;
        mat_inv.inverse();

        return mat_inv;
    }

    void inverse() {

		const ScalarType _m00 = values[0];
		const ScalarType _m10 = values[1];
		const ScalarType _m20 = values[2];
		const ScalarType _m01 = values[3];
		const ScalarType _m11 = values[4];
		const ScalarType _m21 = values[5];
		const ScalarType _m02 = values[6];
		const ScalarType _m12 = values[7];
		const ScalarType _m22 = values[8];

		const float invDet = 1.0f/determinant();

		values[0] = +(_m11*_m22 - _m21*_m12) * invDet;
		values[1] = -(_m10*_m22 - _m20*_m12) * invDet;
		values[2] = +(_m10*_m21 - _m20*_m11) * invDet;

		values[3] = -(_m01*_m22 - _m21*_m02) * invDet;
		values[4] = +(_m00*_m22 - _m20*_m02) * invDet;
		values[5] = -(_m00*_m21 - _m20*_m01) * invDet;

		values[6] = +(_m01*_m12 - _m11*_m02) * invDet;
		values[7] = -(_m00*_m12 - _m10*_m02) * invDet;
		values[8] = +(_m00*_m11 - _m10*_m01) * invDet;
    }

    void set_rotation_x(float rot_rad)
	{
		const auto sx = std::sin(rot_rad);
		const auto cx = std::cos(rot_rad);

		values[ 0] = 1.0f;
		values[ 1] = 0.f;
		values[ 2] = 0.f;

		values[ 3] = 0.f;
		values[ 4] = cx;
		values[ 5] = -sx;

		values[ 6] = 0.f;
		values[ 7] = sx;
		values[ 8] = cx;
	}

	void set_rotation_y(float rot_rad)
	{
		const float sy = std::sin(rot_rad);
		const float cy = std::cos(rot_rad);

		values[0] = cy;
		values[1] = 0.f;
		values[2] = sy;

		values[3] = 0.f;
		values[4] = 1.0f;
		values[5] = 0.f;

		values[6] = -sy;
		values[7] = 0.f;
		values[8] = cy;
	}

	void set_rotation_z(float rot_rad)
	{
		const float sz = sin(rot_rad);
		const float cz = cos(rot_rad);

		values[0] = cz;
		values[1] = -sz;
		values[2] = 0.f;

		values[3] = sz;
		values[4] = cz;
		values[5] = 0.f;

		values[6] = 0.f;
		values[7] = 0.f;
		values[8] = 1.0f;
	}

	void set_rotation_xy(float rotx_rad, float roty_rad)
	{
		const float sx = sin(rotx_rad);
		const float cx = cos(rotx_rad);
		const float sy = sin(roty_rad);
		const float cy = cos(roty_rad);

		values[0] = cy;
		values[1] = 0.f;
		values[2] = sy;

		values[3] = sx*sy;
		values[4] = cx;
		values[5] = -sx*cy;

		values[6] = -cx*sy;
		values[7] = sx;
		values[8] = cx*cy;
	}

	void set_rotation_xyz(float rotx_rad, float roty_rad, float rotz_rad)
	{
		const float sx = sin(rotx_rad);
		const float cx = cos(rotx_rad);
		const float sy = sin(roty_rad);
		const float cy = cos(roty_rad);
		const float sz = sin(rotz_rad);
		const float cz = cos(rotz_rad);

		values[0] = cy*cz;
		values[1] = -cy*sz;
		values[2] = sy;

		values[3] = cz*sx*sy + cx*sz;
		values[4] = cx*cz - sx*sy*sz;
		values[5] = -cy*sx;

		values[6] = -cx*cz*sy + sx*sz;
		values[7] = cz*sx + cx*sy*sz;
		values[8] = cx*cy;
	}

	void set_rotation_zyx(float rotx_rad, float roty_rad, float rotz_rad)
	{
		const float sx = sin(rotx_rad);
		const float cx = cos(rotx_rad);
		const float sy = sin(roty_rad);
		const float cy = cos(roty_rad);
		const float sz = sin(rotz_rad);
		const float cz = cos(rotz_rad);

		values[0] = cy*cz;
		values[1] = cz*sx*sy-cx*sz;
		values[2] = cx*cz*sy+sx*sz;

		values[3] = cy*sz;
		values[4] = cx*cz + sx*sy*sz;
		values[5] = -cz*sx + cx*sy*sz;

		values[6] = -sy;
		values[7] = cy*sx;
		values[8] = cx*cy;
	};

    // Constants

    static inline constexpr Matrix3 indentity() {
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

    static const Matrix3 IDENTITY;
};

// forward declaration required by Clang and MSVC
template <>
const Matrix3<slk::f32> Matrix3<slk::f32>::IDENTITY;
template <>
const Matrix3<slk::i32> Matrix3<slk::i32>::IDENTITY;

template <typename T>
slk::Vector3<T> operator*(slk::Matrix3<T> const& mat, slk::Vector3<T> const& v) {
    return {
        v.x * mat.column0.x + v.y * mat.column1.x + v.z * mat.column2.x, 
        v.x * mat.column0.y + v.y * mat.column1.y + v.z * mat.column2.y,
        v.x * mat.column0.z + v.y * mat.column1.z + v.z * mat.column2.z,
    };
}

template <typename T>
slk::Matrix3<T> operator*(slk::Matrix3<T> const& lval, slk::Matrix3<T> const& rval) {
    return {
        lval * rval.column0,
        lval * rval.column1,
        lval * rval.column2,
    };
}


using Matrix3f = Matrix3<slk::f32>;
using Matrix3i = Matrix3<slk::i32>;

} // namespace slk
