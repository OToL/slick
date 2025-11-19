#include "vector2.hpp"
#include "matrix2.hpp"

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

} // namespace slk
