#pragma once

#include <core/core.hpp>
#include "math.hpp"
#include "vector4.hpp"

namespace slk {

template <typename T>
struct Matrix4 {
    using ParamType = Matrix4Param<T>;
    using RowType = slk::Vector4<T>;
    using RowParamType = typename RowType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            RowType row0;
            RowType row1;
            RowType row2;
            RowType row3;
        };
        RowType rows[4];
        ScalarType values[1];
        struct {
            ScalarType m00, m01, m02, m03;
            ScalarType m10, m11, m12, m13;
            ScalarType m20, m21, m22, m23;
            ScalarType m30, m31, m32, m33;
        };
    };

    Matrix4() = default;

    Matrix4(RowParamType in_row0, RowParamType in_row1, RowParamType in_row2, RowParamType in_row3)
        : row0(in_row0)
        , row1(in_row1)
        , row2(in_row2)
        , row3(in_row3) { }

    Matrix4(ScalarType in_m00, ScalarType in_m01, ScalarType in_m02, ScalarType in_m03,
            ScalarType in_m10, ScalarType in_m11, ScalarType in_m12, ScalarType in_m13,
            ScalarType in_m20, ScalarType in_m21, ScalarType in_m22, ScalarType in_m23,
            ScalarType in_m30, ScalarType in_m31, ScalarType in_m32, ScalarType in_m33)
        : m00(in_m00), m01(in_m01), m02(in_m02), m03(in_m03)
        , m10(in_m10), m11(in_m11), m12(in_m12), m13(in_m13)
        , m20(in_m20), m21(in_m21), m22(in_m22), m23(in_m23)
        , m30(in_m30), m31(in_m31), m32(in_m32), m33(in_m33)
    {}

    // TODO
    // Matrix4& set_rotation(slk::f32 rot_rad) {
    //     row0 = {std::cos(rot_rad), std::sin(rot_rad)};
    //     row1 = {-std::sin(rot_rad), std::cos(rot_rad)};
    //
    //     return *this;
    // }

    void set_identity() {
        row0 = {1, 0, 0, 0};
        row1 = {0, 1, 0, 0};
        row2 = {0, 0, 1, 0};
        row3 = {0, 0, 0, 1};
    }

    [[nodiscard]] ScalarType* data() {
        return &values[0];
    }

    [[nodiscard]] ScalarType const* data() const {
        return &values[0];
    }

    [[nodiscard]] RowType& operator[](slk::u32 row_idx) {
        // TODO: assert on idx out of range
        return rows[row_idx];
    }

    [[nodiscard]] RowType const& operator[](slk::u32 row_idx) const {
        // TODO: assert on idx out of range
        return rows[row_idx];
    }

    // Transforms factory

    // TODO
    // [[nodiscard]] static Matrix4 make_rotation(slk::f32 rot_rad) {
    //     Matrix4 rot_matrix;
    //     rot_matrix.set_rotation(rot_rad);
    //
    //     return rot_matrix;
    // }

    // Constants

    [[nodiscard]] static inline constexpr Matrix4 indentity() {
        return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    }

    static const Matrix4 IDENTITY;
};

// forward declaration required by Clang and MSVC
template <>
const Matrix4<slk::f32> Matrix4<slk::f32>::IDENTITY;
template <>
const Matrix4<slk::i32> Matrix4<slk::i32>::IDENTITY;

template <typename T>
[[nodiscard]] slk::Vector4<T> operator*(slk::Vector4<T> const& v, slk::Matrix4<T> const& mat) {
    return {
        v.x * mat.row0.x + v.y * mat.row1.x + v.z * mat.row2.x + v.w * mat.row3.x, 
        v.x * mat.row0.y + v.y * mat.row1.y + v.z * mat.row2.y + v.w * mat.row3.y,
        v.x * mat.row0.z + v.y * mat.row1.z + v.z * mat.row2.z + v.w * mat.row3.z,
        v.x * mat.row0.w + v.y * mat.row1.w + v.z * mat.row2.w + v.w * mat.row3.w,
    };
}

using Matrix4f = Matrix4<slk::f32>;
using Matrix4i = Matrix4<slk::i32>;

} // namespace slk
