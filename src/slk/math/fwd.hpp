
#include <slk/core.hpp>
#pragma once

namespace slk {

    template <typename T>
    struct Vector2;

    template <typename T>
    struct Vector3;

    template <typename T>
    struct Vector4;

    template <typename T>
    struct Matrix2;

    template <typename T>
    struct Matrix3;

    template <typename T>
    struct Matrix4;

    template <typename T>
    struct AABB2;

    using Vector2i = Vector2<slk::i32>;
    using Vector2f = Vector2<slk::f32>;

    using Vector3i = Vector3<slk::i32>;
    using Vector3f = Vector3<slk::f32>;

    using Vector4i = Vector4<slk::i32>;
    using Vector4f = Vector4<slk::f32>;

    using Matrix2f = Matrix2<slk::f32>;
    using Matrix2i = Matrix2<slk::i32>;

    using Matrix3f = Matrix3<slk::f32>;
    using Matrix3i = Matrix3<slk::i32>;

    using Matrix4f = Matrix4<slk::f32>;
    using Matrix4i = Matrix4<slk::i32>;

    using AABB2i = AABB2<slk::i32>;
    using AABB2f = AABB2<slk::f32>;

}

