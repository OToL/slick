#pragma once

#include <core/core.hpp>
#include "math.hpp"
#include "vector3.hpp"

namespace slk {

template <typename T>
struct Matrix3 {
    using ParamType = Matrix3Param<T>;
    using RowType = slk::Vector3<T>;
    using RowParamType = typename RowType::ParamType;
    using ScalarType = typename RowType::ScalarType;

    union {
        struct {
            RowType row0;
            RowType row1;
            RowType row2;
        };
        RowType rows[3];
        ScalarType values[9];
        struct {
            ScalarType m00, m01, m02;
            ScalarType m10, m11, m12;
            ScalarType m20, m21, m22;
        };
    };

    Matrix3() = default;

    Matrix3(RowParamType in_row0, RowParamType in_row1, RowParamType in_row2)
        : row0(in_row0)
        , row1(in_row1)
        , row2(in_row2) { }

    Matrix3(ScalarType in_m00, ScalarType in_m01, ScalarType in_m02,
            ScalarType in_m10, ScalarType in_m11, ScalarType in_m12,
            ScalarType in_m20, ScalarType in_m21, ScalarType in_m22)
        : m00(in_m00), m01(in_m01), m02(in_m02)
        , m10(in_m10), m11(in_m11), m12(in_m12)
        , m20(in_m20), m21(in_m21), m22(in_m22)
    {}

    // TODO
    // Matrix3& set_rotation(slk::f32 rot_rad) {
    //     row0 = {std::cos(rot_rad), std::sin(rot_rad)};
    //     row1 = {-std::sin(rot_rad), std::cos(rot_rad)};
    //
    //     return *this;
    // }

    void set_identity() {
        row0 = {1, 0, 0};
        row1 = {0, 1, 0};
        row2 = {0, 0, 1};
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
    // [[nodiscard]] static Matrix3 make_rotation(slk::f32 rot_rad) {
    //     Matrix3 rot_matrix;
    //     rot_matrix.set_rotation(rot_rad);
    //
    //     return rot_matrix;
    // }

    // Constants

    [[nodiscard]] static inline constexpr Matrix3 indentity() {
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
[[nodiscard]] slk::Vector3<T> operator*(slk::Vector3<T> const& v, slk::Matrix3<T> const& mat) {
    return {
        v.x * mat.row0.x + v.y * mat.row1.x + v.z * mat.row2.x, 
        v.x * mat.row0.y + v.y * mat.row1.y + v.z * mat.row2.y,
        v.x * mat.row0.z + v.y * mat.row1.z + v.z * mat.row2.z,
    };
}

using Matrix3f = Matrix3<slk::f32>;
using Matrix3i = Matrix3<slk::i32>;

} // namespace slk
