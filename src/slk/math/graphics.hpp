#pragma once

#include "matrix4.hpp"
#include "fwd.hpp"

namespace slk {

Matrix4f makeOrthoProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, f32 offset, bool homogeneous_ndc,
                               Handedness handedness = Handedness::LEFT);

Matrix4f makeLookAtMatrix(const Vector3f& eye, const Vector3f& at, const Vector3f& ref_up, Handedness handedness = Handedness::LEFT);

Matrix4f makePerspectiveProjectionMatrix(f32 fovy_rad, f32 aspect, f32 near, f32 far, bool homogeneousNdc, Handedness handedness = Handedness::LEFT);

} // namespace slk
