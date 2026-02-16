#pragma once

#include <slk/core.hpp>
#include "math.hpp"
#include "vector4.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"

namespace slk {

template <typename T>
struct Matrix4 {
    using ParamType = Matrix4Param<T>;
    using RowType = slk::Vector4<T>;
    using ColumnType = slk::Vector4<T>;
    using RowParamType = typename RowType::ParamType;
    using ColumnParamType = typename ColumnType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            ColumnType column0;
            ColumnType column1;
            ColumnType column2;
            ColumnType column3;
        };
        ColumnType columns[4];
        ScalarType values[16];
        struct {
            ScalarType m00, m10, m20, m30;
            ScalarType m01, m11, m21, m31;
            ScalarType m02, m12, m22, m32;
            ScalarType m03, m13, m23, m33;
        };
    };

    Matrix4() = default;

    Matrix4(RowParamType _row0, RowParamType _row1, RowParamType _row2, RowParamType _row3)
        : column0(_row0)
        , column1(_row1)
        , column2(_row2)
        , column3(_row3) { }

    Matrix4(ScalarType _m00, ScalarType _m01, ScalarType _m02, ScalarType _m03,
            ScalarType _m10, ScalarType _m11, ScalarType _m12, ScalarType _m13,
            ScalarType _m20, ScalarType _m21, ScalarType _m22, ScalarType _m23,
            ScalarType _m30, ScalarType _m31, ScalarType _m32, ScalarType _m33)
        : m00(_m00), m10(_m10), m20(_m20), m30(_m30)
        , m01(_m01), m11(_m11), m21(_m21), m31(_m31)
        , m02(_m02), m12(_m12), m22(_m22), m32(_m32)
        , m03(_m03), m13(_m13), m23(_m23), m33(_m33)
    {}

    Matrix4& setIdentity() {
        column0 = {1, 0, 0, 0};
        column1 = {0, 1, 0, 0};
        column2 = {0, 0, 1, 0};
        column3 = {0, 0, 0, 1};

        return *this;
    }

    Matrix4& transpose()
    {
        for (u32 col_idx = 0 ; col_idx != 4 ; ++col_idx) {
            for (u32 row_idx = col_idx + 1; row_idx != 4; ++row_idx)
            {
                const ScalarType tmp = columns[col_idx][row_idx];
                columns[col_idx][row_idx] = columns[row_idx][col_idx];
                columns[row_idx][col_idx] = tmp;
            }
        }

        return *this;
    }

    Matrix4 transposed() const {
        return {
            columns[0], columns[4], columns[ 8], columns[12],
            columns[1], columns[5], columns[ 9], columns[13],
            columns[2], columns[6], columns[10], columns[14],
            columns[3], columns[7], columns[11], columns[15],
        };
    }

	void setPerspectiveProj(f32 _fovy_rad, f32 _aspect, f32 _near, f32 _far, bool _homogeneousNdc, EHandedness _handedness = EHandedness::Left)
	{
		const f32 height = 1.0f/std::tan(_fovy_rad*0.5f);
		const f32 width  = height * 1.0f/_aspect;
		setPerspectiveProjXYWH(0.0f, 0.0f, width, height, _near, _far, _homogeneousNdc, _handedness);
	}

	void setOrthoProje(f32 _left, f32 _right, f32 _bottom, f32 _top, f32 _near, f32 _far, f32 _offset, bool _homogeneousNdc, EHandedness _handedness = EHandedness::Left)
	{
		const f32 aa = 2.0f/(_right - _left);
		const f32 bb = 2.0f/(_top - _bottom);
		const f32 cc = (_homogeneousNdc ? 2.0f : 1.0f) / (_far - _near);
		const f32 dd = (_left + _right )/(_left   - _right);
		const f32 ee = (_top  + _bottom)/(_bottom - _top  );
		const f32 ff = _homogeneousNdc
			? (_near + _far)/(_near - _far)
			:  _near        /(_near - _far);

		values[ 0] = aa;
		values[ 1] = 0.f;
		values[ 2] = 0.f;
		values[ 3] = 0.f;

		values[ 4] = 0.f;
		values[ 5] = bb;
		values[ 6] = 0.f;
		values[ 7] = 0.f;

		values[ 8] = 0.f;
		values[ 9] = 0.f;
		values[10] = EHandedness::Right == _handedness ? -cc : cc;
		values[11] = 0.f;

		values[12] = dd + _offset;
		values[13] = ee;
		values[14] = ff;
		values[15] = 1.0f;
	}

	void setLookAt(const Vector3f& eye, const Vector3f& at, const Vector3f& ref_up, EHandedness handedness = EHandedness::Left)
	{
		const Vector3f view = (EHandedness::Right == handedness ? eye - at : at - eye).normalized();

		Vector3f right = ref_up.cross(view);
		if (0.0f == right.dot(right) )
		{
			right = { EHandedness::Left == handedness ? -1.0f : 1.0f, 0.0f, 0.0f };
		}
		else
		{
			right.normalize();
		}

		const Vector3f up = view.cross(right);

		values[ 0] = right.x;
		values[ 1] = up.x;
		values[ 2] = view.x;
		values[ 3] = 0.0f;

		values[ 4] = right.y;
		values[ 5] = up.y;
		values[ 6] = view.y;
		values[ 7] = 0.0f;

		values[ 8] = right.z;
		values[ 9] = up.z;
		values[10] = view.z;
		values[11] = 0.0f;

		values[12] = -right.dot(eye);
		values[13] = -up.dot(eye);
		values[14] = -view.dot(eye);
		values[15] = 1.0f;
	}

    ScalarType* data() {
        return &values[0];
    }

    ScalarType const* data() const {
        return &values[0];
    }

    ColumnType getColumn(u32 idx) const {
        return columns[idx];
    }

    RowType getRow(u32 idx) const {
        return {columns[0][idx], columns[1][idx], columns[2][idx], 1.f};
    }

    RowType getTranslation() const {
        return column3;
    }

    void setTranslation(RowParamType translation) {
        column3 = translation;
    }

    void setTranslation(Vector3<ScalarType> translation) {
        column3 = {translation.x, translation.y, translation.z, 1.f};
    }

    slk::Matrix3<ScalarType> getRotation() const {
        return {
            m00, m01, m02,
            m10, m11, m12,
            m20, m21, m22
        };
    }

    void setRotration(Matrix3<ScalarType>::ParamType rot) const {
        column0 = {rot.column0.x, rot.column0.y, rot.column0.z, 0.f};
        column1 = {rot.column1.x, rot.column1.y, rot.column1.z, 0.f};
        column2 = {rot.column2.x, rot.column2.y, rot.column2.z, 0.f};
    }

    ScalarType getValue(u32 row_idx, u32 col_idx) const {
        return columns[col_idx][row_idx];
    }

    void setValue(u32 row_idx, u32 col_idx, ScalarType val) const {
        columns[col_idx][row_idx] = val;
    }

    ColumnType getXAxis() const {
        return {column0.x, column0.y, column0.z, 1.f};
    }

    ColumnType getYAxis() const {
        return {column1.x, column1.y, column1.z, 1.f};
    }

    ColumnType getZAxis() const {
        return {column2.x, column2.y, column2.z, 1.f};
    }

    // Sign matrix used for cofactor matrix and determinant
    //
    //  [+ - + -]
    //  [- + - +]
    //  [+ - + -]
    //  [- + - +]
    //
    ScalarType determinant() const {
        return m00 * (m11*(m22*m33 - m32*m23) - m21*(m12*m33 - m32*m13) + m31*(m12*m23 - m22*m13) )
            - m10 * (m01*(m22*m33 - m32*m23) - m21*(m02*m33 - m32*m03) + m31*(m02*m23 - m22*m03) )
            + m20 * (m01*(m12*m33 - m32*m13) - m11*(m02*m33 - m32*m03) + m31*(m02*m13 - m12*m03) )
            - m30 * (m01*(m12*m23 - m22*m13) - m11*(m02*m23 - m22*m03) + m21*(m02*m13 - m12*m03) );
    }

    void inverse() {
		const f32 xx = values[ 0];
		const f32 xy = values[ 1];
		const f32 xz = values[ 2];
		const f32 xw = values[ 3];
		const f32 yx = values[ 4];
		const f32 yy = values[ 5];
		const f32 yz = values[ 6];
		const f32 yw = values[ 7];
		const f32 zx = values[ 8];
		const f32 zy = values[ 9];
		const f32 zz = values[10];
		const f32 zw = values[11];
		const f32 wx = values[12];
		const f32 wy = values[13];
		const f32 wz = values[14];
		const f32 ww = values[15];

		f32 det = 0.0f;
		det += xx * (yy*(zz*ww - zw*wz) - yz*(zy*ww - zw*wy) + yw*(zy*wz - zz*wy) );
		det -= xy * (yx*(zz*ww - zw*wz) - yz*(zx*ww - zw*wx) + yw*(zx*wz - zz*wx) );
		det += xz * (yx*(zy*ww - zw*wy) - yy*(zx*ww - zw*wx) + yw*(zx*wy - zy*wx) );
		det -= xw * (yx*(zy*wz - zz*wy) - yy*(zx*wz - zz*wx) + yz*(zx*wy - zy*wx) );

		f32 inv_det = 1.0f/det;

		values[ 0] = +(yy*(zz*ww - wz*zw) - yz*(zy*ww - wy*zw) + yw*(zy*wz - wy*zz) ) * inv_det;
		values[ 1] = -(xy*(zz*ww - wz*zw) - xz*(zy*ww - wy*zw) + xw*(zy*wz - wy*zz) ) * inv_det;
		values[ 2] = +(xy*(yz*ww - wz*yw) - xz*(yy*ww - wy*yw) + xw*(yy*wz - wy*yz) ) * inv_det;
		values[ 3] = -(xy*(yz*zw - zz*yw) - xz*(yy*zw - zy*yw) + xw*(yy*zz - zy*yz) ) * inv_det;

		values[ 4] = -(yx*(zz*ww - wz*zw) - yz*(zx*ww - wx*zw) + yw*(zx*wz - wx*zz) ) * inv_det;
		values[ 5] = +(xx*(zz*ww - wz*zw) - xz*(zx*ww - wx*zw) + xw*(zx*wz - wx*zz) ) * inv_det;
		values[ 6] = -(xx*(yz*ww - wz*yw) - xz*(yx*ww - wx*yw) + xw*(yx*wz - wx*yz) ) * inv_det;
		values[ 7] = +(xx*(yz*zw - zz*yw) - xz*(yx*zw - zx*yw) + xw*(yx*zz - zx*yz) ) * inv_det;

		values[ 8] = +(yx*(zy*ww - wy*zw) - yy*(zx*ww - wx*zw) + yw*(zx*wy - wx*zy) ) * inv_det;
		values[ 9] = -(xx*(zy*ww - wy*zw) - xy*(zx*ww - wx*zw) + xw*(zx*wy - wx*zy) ) * inv_det;
		values[10] = +(xx*(yy*ww - wy*yw) - xy*(yx*ww - wx*yw) + xw*(yx*wy - wx*yy) ) * inv_det;
		values[11] = -(xx*(yy*zw - zy*yw) - xy*(yx*zw - zx*yw) + xw*(yx*zy - zx*yy) ) * inv_det;

		values[12] = -(yx*(zy*wz - wy*zz) - yy*(zx*wz - wx*zz) + yz*(zx*wy - wx*zy) ) * inv_det;
		values[13] = +(xx*(zy*wz - wy*zz) - xy*(zx*wz - wx*zz) + xz*(zx*wy - wx*zy) ) * inv_det;
		values[14] = -(xx*(yy*wz - wy*yz) - xy*(yx*wz - wx*yz) + xz*(yx*wy - wx*yy) ) * inv_det;
		values[15] = +(xx*(yy*zz - zy*yz) - xy*(yx*zz - zx*yz) + xz*(yx*zy - zx*yy) ) * inv_det;
    }

	void setRotationX(f32 rot_rad)
	{
		const f32 sx = sin(rot_rad);
		const f32 cx = cos(rot_rad);

		values[ 0] = 1.0f;
		values[ 1] = 0.f;
		values[ 2] = 0.f;
		values[ 3] = 0.f;

		values[ 4] = 0.f;
		values[ 5] = cx;
		values[ 6] = -sx;
		values[ 7] = 0.f;

		values[ 8] = 0.f;
		values[ 9] = sx;
		values[10] = cx;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	}

	void setRotationY(f32 rot_rad)
	{
		const f32 sy = sin(rot_rad);
		const f32 cy = cos(rot_rad);

		values[ 0] = cy;
		values[ 1] = 0.f;
		values[ 2] = sy;
		values[ 3] = 0.f;

		values[ 4] = 0.f;
		values[ 5] = 1.0f;
		values[ 6] = 0.f;
		values[ 7] = 0.f;

		values[ 8] = -sy;
		values[ 9] = 0.f;
		values[10] = cy;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	}

	void setRotationZ(f32 rot_rad)
	{
		const f32 sz = sin(rot_rad);
		const f32 cz = cos(rot_rad);

		values[ 0] = cz;
		values[ 1] = -sz;
		values[ 2] = 0.f;
		values[ 3] = 0.f;

		values[ 4] = sz;
		values[ 5] = cz;
		values[ 6] = 0.f;
		values[ 7] = 0.f;

		values[ 8] = 0.f;
		values[ 9] = 0.f;
		values[10] = 1.0f;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	}

	void setRotationXY(f32 rotx_rad, f32 roty_rad)
	{
		const f32 sx = sin(rotx_rad);
		const f32 cx = cos(rotx_rad);
		const f32 sy = sin(roty_rad);
		const f32 cy = cos(roty_rad);

		values[ 0] = cy;
		values[ 1] = 0.f;
		values[ 2] = sy;
		values[ 3] = 0.f;

		values[ 4] = sx*sy;
		values[ 5] = cx;
		values[ 6] = -sx*cy;
		values[ 7] = 0.f;

		values[ 8] = -cx*sy;
		values[ 9] = sx;
		values[10] = cx*cy;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	}

	void setRotationXYZ(f32 rotx_rad, f32 roty_rad, f32 rotz_rad)
	{
		const f32 sx = sin(rotx_rad);
		const f32 cx = cos(rotx_rad);
		const f32 sy = sin(roty_rad);
		const f32 cy = cos(roty_rad);
		const f32 sz = sin(rotz_rad);
		const f32 cz = cos(rotz_rad);

		values[ 0] = cy*cz;
		values[ 1] = -cy*sz;
		values[ 2] = sy;
		values[ 3] = 0.f;

		values[ 4] = cz*sx*sy + cx*sz;
		values[ 5] = cx*cz - sx*sy*sz;
		values[ 6] = -cy*sx;
		values[ 7] = 0.f;

		values[ 8] = -cx*cz*sy + sx*sz;
		values[ 9] = cz*sx + cx*sy*sz;
		values[10] = cx*cy;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	}

	void setRotationZYX(f32 rotx_rad, f32 roty_rad, f32 rotz_rad)
	{
		const f32 sx = sin(rotx_rad);
		const f32 cx = cos(rotx_rad);
		const f32 sy = sin(roty_rad);
		const f32 cy = cos(roty_rad);
		const f32 sz = sin(rotz_rad);
		const f32 cz = cos(rotz_rad);

		values[ 0] = cy*cz;
		values[ 1] = cz*sx*sy-cx*sz;
		values[ 2] = cx*cz*sy+sx*sz;
		values[ 3] = 0.f;

		values[ 4] = cy*sz;
		values[ 5] = cx*cz + sx*sy*sz;
		values[ 6] = -cz*sx + cx*sy*sz;
		values[ 7] = 0.f;

		values[ 8] = -sy;
		values[ 9] = cy*sx;
		values[10] = cx*cy;
		values[11] = 0.f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = 0.f;
		values[15] = 1.0f;
	};


    // Constants

    static inline constexpr Matrix4 indentity() {
        return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    }

    static const Matrix4 IDENTITY;

private:

    void setPerspectiveProjXYWH(f32 _x, f32 _y, f32 _width, f32 _height, f32 _near, f32 _far, bool _homogeneousNdc, EHandedness _handedness)
	{
		const f32 diff = _far-_near;
		const f32 aa = _homogeneousNdc ? (     _far+_near)/diff : _far/diff;
		const f32 bb = _homogeneousNdc ? (2.0f*_far*_near)/diff : _near*aa;

		values[ 0] = _width;
		values[ 1] = 0.f;
		values[ 2] = 0.f;
		values[ 3] = 0.f;

		values[ 4] = 0.f;
		values[ 5] = _height;
		values[ 6] = 0.f;
		values[ 7] = 0.f;

		values[ 8] = (EHandedness::Right == _handedness) ?    _x :  -_x;
		values[ 9] = (EHandedness::Right == _handedness) ?    _y :  -_y;
		values[10] = (EHandedness::Right == _handedness) ?   -aa :   aa;
		values[11] = (EHandedness::Right == _handedness) ? -1.0f : 1.0f;

		values[12] = 0.f;
		values[13] = 0.f;
		values[14] = -bb;
		values[15] = 0.f;
	}
};

// forward declaration required by Clang and MSVC
template <>
const Matrix4<slk::f32> Matrix4<slk::f32>::IDENTITY;
template <>
const Matrix4<slk::i32> Matrix4<slk::i32>::IDENTITY;

template <typename T>
slk::Vector4<T> operator*(slk::Matrix4<T> const& mat, slk::Vector4<T> const& v) {
    return {
        v.x * mat.column0.x + v.y * mat.column1.x + v.z * mat.column2.x + v.w * mat.column3.x, 
        v.x * mat.column0.y + v.y * mat.column1.y + v.z * mat.column2.y + v.w * mat.column3.y,
        v.x * mat.column0.z + v.y * mat.column1.z + v.z * mat.column2.z + v.w * mat.column3.z,
        v.x * mat.column0.w + v.y * mat.column1.w + v.z * mat.column2.w + v.w * mat.column3.w,
    };
}

template <typename T>
slk::Matrix4<T> operator*(slk::Matrix4<T> const& lval, slk::Matrix4<T> const& rval) {
    return {
        lval * rval.column0,
        lval * rval.column1,
        lval * rval.column2,
        lval * rval.column3
    };
}

using Matrix4f = Matrix4<slk::f32>;
using Matrix4i = Matrix4<slk::i32>;

} // namespace slk
