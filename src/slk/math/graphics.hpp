#pragma once

#include "matrix4.hpp"
#include "vector2.hpp"
#include "fwd.hpp"

namespace slk {

Matrix4f makeOrthoProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, f32 offset, bool homogeneous_ndc,
                               Handedness handedness = Handedness::LEFT);

Matrix4f makeLookAtMatrix(Vector3f eye, Vector3f at, Vector3f ref_up, Handedness handedness = Handedness::LEFT);

Matrix4f makePerspectiveProjectionMatrix(f32 fovy_rad, f32 aspect, f32 near, f32 far, bool homogeneousNdc, Handedness handedness = Handedness::LEFT);

// screen_pos: [0 .. screen_dimension.xy] with x points right and y points down
// returns NDC coordinates [-1 .. 1] with x points right and y points up
Vector2f screenPosToNdc(Vector2f screen_pos, Vector2f screen_dimension);

} // namespace slk
