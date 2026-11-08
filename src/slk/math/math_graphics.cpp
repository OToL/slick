#include "graphics.hpp"
#include "math.hpp"

namespace slk {

Vector2f screenPosToNdc(Vector2f screen_pos, Vector2f screen_dimension)
{
    return {
        2.f * (screen_pos.m_x / screen_dimension.m_x) - 1.f,
        1.f - 2.f * (screen_pos.m_y / screen_dimension.m_y)
    };
}

Matrix4f makeOrthoProjectionMatrix(OrthoProjectionParams const& params, f32 near, f32 far, bool homogeneous_ndc,
                               EHandedness handedness) {
    const f32 aa = 2.0f / (params.m_right - params.m_left);
    const f32 bb = 2.0f / (params.m_top - params.m_bottom);
    const f32 cc = (homogeneous_ndc ? 2.0f : 1.0f) / (far - near);
    const f32 dd = (params.m_left + params.m_right) / (params.m_left - params.m_right);
    const f32 ee = (params.m_top + params.m_bottom) / (params.m_bottom - params.m_top);
    const f32 ff = homogeneous_ndc ? (near + far) / (near - far) : near / (near - far);

    // Matrix4 scalar constructor takes columns
    return {
        aa, 0.f, 0.f, 0.f,
        0.f, bb, 0.f, 0.f,
        0.f, 0.f, EHandedness::RIGHT == handedness ? -cc : cc, 0.f,
        dd + params.m_offset, ee, ff, 1.0f,
    };
}

Matrix4f makeLookAtMatrix(Vector3f eye, Vector3f at, Vector3f ref_up, EHandedness handedness) {
    const Vector3f view = (EHandedness::RIGHT == handedness ? eye - at : at - eye).normalized();

    Vector3f right = ref_up.cross(view);
    if (0.0f == right.dot(right)) {
        right = {EHandedness::LEFT == handedness ? -1.0f : 1.0f, 0.0f, 0.0f};
    } else {
        right.normalize();
    }

    const Vector3f up = view.cross(right);

    return {
        right.m_x, up.m_x, view.m_x, 0.0f, right.m_y, up.m_y, view.m_y, 0.0f, right.m_z, up.m_z, view.m_z, 0.0f, -right.dot(eye), -up.dot(eye), -view.dot(eye), 1.0f,
    };
}

Matrix4f make_perspective_projection_xywh(f32 x, f32 y, f32 width, f32 height, f32 near, f32 far, bool homogeneousNdc, EHandedness handedness)
{
    const f32 diff = far-near;
    const f32 aa = homogeneousNdc ? (     far+near)/diff : far/diff;
    const f32 bb = homogeneousNdc ? (2.0f*far*near)/diff : near*aa;

    return {
        width,
        0.f,
        0.f,
        0.f,

        0.f,
        height,
        0.f,
        0.f,

        (EHandedness::RIGHT == handedness) ?    x :  -x,
        (EHandedness::RIGHT == handedness) ?    y :  -y,
        (EHandedness::RIGHT == handedness) ?   -aa :   aa,
        (EHandedness::RIGHT == handedness) ? -1.0f : 1.0f,

        0.f,
        0.f,
        -bb,
        0.f,
    };
}

Matrix4f makePerspectiveProjectionMatrix(PerspectiveProjectionParams const& params, f32 near, f32 far, bool homogeneousNdc, EHandedness handedness)
{
    const f32 height = 1.0f/std::tan(params.m_fovy*0.5f);
    const f32 width  = height * 1.0f/params.m_aspect;
    return make_perspective_projection_xywh(0.0f, 0.0f, width, height, near, far, homogeneousNdc, handedness);
}

} // namespace slk
