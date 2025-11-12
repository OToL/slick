#pragma once

#include <core/core.hpp>

namespace slk {

    template <typename T>
    struct Vector2;

    template <typename T>
    struct Matrix2;

    template <typename T>
    struct AABB2;

    using Vector2i = Vector2<slk::i32>;
    using Vector2f = Vector2<slk::f32>;

    using Matrix2f = Matrix2<slk::f32>;
    using Matrix2i = Matrix2<slk::i32>;

    using AABB2i = AABB2<slk::i32>;
    using AABB2f = AABB2<slk::f32>;

}


