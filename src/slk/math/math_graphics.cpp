#include "graphics.hpp"

slk::Matrix4f slk::make_ortho_projection(f32 _left, f32 _right, f32 _bottom, f32 _top, f32 _near, f32 _far, f32 _offset, bool _homogeneousNdc, EHandedness _handedness)
{
    const f32 aa = 2.0f/(_right - _left);
    const f32 bb = 2.0f/(_top - _bottom);
    const f32 cc = (_homogeneousNdc ? 2.0f : 1.0f) / (_far - _near);
    const f32 dd = (_left + _right )/(_left   - _right);
    const f32 ee = (_top  + _bottom)/(_bottom - _top  );
    const f32 ff = _homogeneousNdc
        ? (_near + _far)/(_near - _far)
        :  _near        /(_near - _far);

    return {
        aa,     0.f,    0.f,    dd + _offset,
        0.f,    bb,     0.f,    ee,
        0.f,    0.f,    EHandedness::Right == _handedness ? -cc : cc,   ff,
        0.f,    0.f,    0.f,   1.0f,
    };
}

slk::Matrix4f slk::make_look_at(const Vector3f& eye, const Vector3f& at, const Vector3f& ref_up, EHandedness handedness)
{
    const Vector3f view = (EHandedness::Right == handedness ? eye - at : at - eye).normalized();

    Vector3f right = ref_up.cross(view);
    if (0.0f == right.dot(right)) {
        right = {EHandedness::Left == handedness ? -1.0f : 1.0f, 0.0f, 0.0f};
    } else {
        right.normalize();
    }

    const Vector3f up = view.cross(right);

    return {
        right.x, up.x, view.x, 0.0f,
        right.y, up.y, view.y, 0.0f,
        right.z, up.z, view.z, 0.0f,
        -right.dot(eye), -up.dot(eye), -view.dot(eye), 1.0f,
    };
}
