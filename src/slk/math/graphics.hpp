#pragma once

#include "math.hpp"
#include "matrix4.hpp"
#include "fwd.hpp"

namespace slk {

Matrix4f make_ortho_projection(f32 _left, f32 _right, f32 _bottom, f32 _top, f32 _near, f32 _far, f32 _offset, bool _homogeneousNdc,
                               EHandedness _handedness = EHandedness::Left);

Matrix4f make_look_at(const Vector3f& eye, const Vector3f& at, const Vector3f& ref_up, EHandedness handedness = EHandedness::Left);

} // namespace slk
