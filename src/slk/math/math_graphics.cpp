#include "graphics.hpp"

namespace slk {

Matrix4f makeOrthoProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, f32 offset, bool homogeneous_ndc,
                               Handedness handedness) {
    const f32 aa = 2.0f / (right - left);
    const f32 bb = 2.0f / (top - bottom);
    const f32 cc = (homogeneous_ndc ? 2.0f : 1.0f) / (far - near);
    const f32 dd = (left + right) / (left - right);
    const f32 ee = (top + bottom) / (bottom - top);
    const f32 ff = homogeneous_ndc ? (near + far) / (near - far) : near / (near - far);

    return {
        aa, 0.f, 0.f, dd + offset, 0.f, bb, 0.f, ee, 0.f, 0.f, Handedness::RIGHT == handedness ? -cc : cc, ff, 0.f, 0.f, 0.f, 1.0f,
    };
}

Matrix4f makeLookAtMatrix(const Vector3f& eye, const Vector3f& at, const Vector3f& ref_up, Handedness handedness) {
    const Vector3f view = (Handedness::RIGHT == handedness ? eye - at : at - eye).normalized();

    Vector3f right = ref_up.cross(view);
    if (0.0f == right.dot(right)) {
        right = {Handedness::LEFT == handedness ? -1.0f : 1.0f, 0.0f, 0.0f};
    } else {
        right.normalize();
    }

    const Vector3f up = view.cross(right);

    return {
        right.m_x, up.m_x, view.m_x, 0.0f, right.m_y, up.m_y, view.m_y, 0.0f, right.m_z, up.m_z, view.m_z, 0.0f, -right.dot(eye), -up.dot(eye), -view.dot(eye), 1.0f,
    };
}

Matrix4f make_perspective_projection_xywh(f32 x, f32 y, f32 width, f32 height, f32 near, f32 far, bool homogeneousNdc, Handedness handedness)
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

        (Handedness::RIGHT == handedness) ?    x :  -x,
        (Handedness::RIGHT == handedness) ?    y :  -y,
        (Handedness::RIGHT == handedness) ?   -aa :   aa,
        (Handedness::RIGHT == handedness) ? -1.0f : 1.0f,

        0.f,
        0.f,
        -bb,
        0.f,
    };
}

Matrix4f makePerspectiveProjectionMatrix(f32 fovy_rad, f32 aspect, f32 near, f32 far, bool homogeneousNdc, Handedness handedness)
{
    const f32 height = 1.0f/std::tan(fovy_rad*0.5f);
    const f32 width  = height * 1.0f/aspect;
    return make_perspective_projection_xywh(0.0f, 0.0f, width, height, near, far, homogeneousNdc, handedness);
}

} // namespace slk
