#pragma once

#include "matrix4.hpp"
#include "vector2.hpp"
#include "fwd.hpp"

namespace slk {

Matrix4f makeOrthoProjectionMatrix(OrthoProjectionParams const& params, f32 znear, f32 zfar, bool homogeneous_ndc, EHandedness handedness = EHandedness::LEFT);

Matrix4f makeLookAtMatrix(Vector3f eye, Vector3f at, Vector3f ref_up, EHandedness handedness = EHandedness::LEFT);

Matrix4f makePerspectiveProjectionMatrix(PerspectiveProjectionParams const& params, f32 znear, f32 zfar, bool homogeneousNdc,
                                         EHandedness handedness = EHandedness::LEFT);

// screen_pos: [0 .. screen_dimension.xy] with x points right and y points down
// returns NDC coordinates [-1 .. 1] with x points right and y points up
Vector2f screenPosToNdc(Vector2f screen_pos, Vector2f screen_dimension);

} // namespace slk
