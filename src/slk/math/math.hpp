#pragma once

#include <slk/core.hpp>

//
// Conventions ...
// - Column Major matrix layout: This is the standard for most Render API/RHI (OpenGL, Vulkan, Bgfx), math libraries (Havok, SONY vector math) and academic publications
// - Right to left composition (M2.M1.v): API and literature using column major layout for matrix are using this composition order
// - Column vector
// - Left Handedness: In terms of camera/projection it means +Z is along the view vector
//

namespace slk {

template <typename T>
struct Vector2;
template <typename T>
struct Vector3;
template <typename T>
struct Vector4;
template <typename T>
struct AABB2;
template <typename T>
struct Matrix2;
template <typename T>
struct Matrix3;
template <typename T>
struct Matrix4;

template<typename T>
using Vector2Param = Vector2<T>;
template<typename T>
using Vector3Param = Vector3<T>;
template<typename T>
using Vector4Param = Vector4<T> const&;
template<typename T>
using AABB2Param = AABB2<T> const&;
template<typename T>
using Matrix2Param = Matrix2<T>;
template<typename T>
using Matrix3Param = Matrix3<T> const&;
template<typename T>
using Matrix4Param = Matrix4<T> const&;

inline constexpr f32 PI_F32 = 3.14159265f;
inline constexpr f64 PI_F64 = 3.14159265358979323846;

enum class Handedness : u8
{
    LEFT,
    RIGHT,

    DEFAULT = LEFT
};

enum class Projection : u8
{
    PERSPECTIVE,
    ORTHO,
};

} // namespace slk
