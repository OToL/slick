#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include "matrix2.hpp"
#include "matrix3.hpp"
#include "matrix4.hpp"

namespace slk {

template <>
const Vector2f Vector2<f32>::ZERO{0.f, 0.f};

template <>
const Vector2f Vector2<f32>::UNITX{1.f, 0.f};

template <>
const Vector2f Vector2<f32>::UNITY{0.f, 1.f};

template <>
const Vector2i Vector2<i32>::ZERO{0, 0};

template <>
const Vector2i Vector2<i32>::UNITX{1, 0};

template <>
const Vector2i Vector2<i32>::UNITY{0, 1};

template <>
const Vector3f Vector3<f32>::ZERO{0.f, 0.f, 0.f};

template <>
const Vector3f Vector3<f32>::UNITX{1.f, 0.f, 0.f};

template <>
const Vector3f Vector3<f32>::UNITY{0.f, 1.f, 0.f};

template <>
const Vector3f Vector3<f32>::UNITZ{0.f, 0.f, 1.f};

template <>
const Vector3i Vector3<i32>::ZERO{0, 0, 0};

template <>
const Vector3i Vector3<i32>::UNITX{1, 0, 0};

template <>
const Vector3i Vector3<i32>::UNITY{0, 1, 0};

template <>
const Vector3i Vector3<i32>::UNITZ{0, 0, 1};

template <>
const Vector4f Vector4<f32>::ZERO{0.f, 0.f, 0.f, 0.f};

template <>
const Vector4f Vector4<f32>::UNITX{1.f, 0.f, 0.f, 0.f};

template <>
const Vector4f Vector4<f32>::UNITY{0.f, 1.f, 0.f, 0.f};

template <>
const Vector4f Vector4<f32>::UNITZ{0.f, 0.f, 1.f, 0.f};

template <>
const Vector4f Vector4<f32>::UNITW{0.f, 0.f, 0.f, 1.f};

template <>
const Vector4i Vector4<i32>::ZERO{0, 0, 0, 0};

template <>
const Vector4i Vector4<i32>::UNITX{1, 0, 0, 0};

template <>
const Vector4i Vector4<i32>::UNITY{0, 1, 0, 0};

template <>
const Vector4i Vector4<i32>::UNITZ{0, 0, 1, 0};

template <>
const Vector4i Vector4<i32>::UNITW{0, 0, 0, 1};

template<> 
const Matrix2f Matrix2<slk::f32>::IDENTITY = {
    {1, 0},
    {0, 1}
};

template<> 
const Matrix2i Matrix2<slk::i32>::IDENTITY = {
    {1, 0},
    {0, 1}
};

template<> 
const Matrix3f Matrix3<slk::f32>::IDENTITY = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
};

template<> 
const Matrix3i Matrix3<slk::i32>::IDENTITY = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
};

template<> 
const Matrix4f Matrix4<slk::f32>::IDENTITY = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

template<> 
const Matrix4i Matrix4<slk::i32>::IDENTITY = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

} // namespace slk
