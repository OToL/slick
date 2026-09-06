#pragma once

#include <slk/math/vector3.hpp>
#include <slk/math/math.hpp>
#include <slk/math/matrix3.hpp>
#include <slk/math/fwd.hpp>
#include "dispatch_tag.hpp"

namespace slk {

// TODO: orbit/target
// TODO: zoom
struct Camera {

    Camera();
    Camera(slk::EnumTag<Projection::ORTHO>, f32 near, f32 far, f32 left, f32 right, f32 top, f32 bottom, f32 offset);
    Camera(slk::EnumTag<Projection::PERSPECTIVE>, f32 near, f32 far, f32 fovy, f32 aspect);

    // axis
    Vector3f right() const;
    Vector3f up() const;
    Vector3f forward() const;

    // movements
    void rotate(f32 delta_yaw, f32 delta_pitch);
    void translate(Vector3f const& vec);
    void pan(f32 delta_x, f32 delta_y);

    // view/proj
    void setLookAt(Vector3f eye, Vector3f target);
    Matrix4f viewMatrix() const;

    void setPerspectiveProjection(f32 near, f32 far, f32 fovy, f32 aspect);
    void setOrthoProjection(f32 near, f32 far, f32 left, f32 right, f32 top, f32 bottom, f32 offset);
    Matrix4f projectionMatrix(Handedness hand, bool homogeneous_ndc) const;

    Matrix3f m_orientation;
    Vector3f m_pos;
    f32 m_near;
    f32 m_far;
    Projection m_projection;

    union {
        struct {
            float m_fovy;
            float m_aspect;
        };

        struct {
            float m_left;
            float m_right;
            float m_top;
            float m_bottom;
            float m_offset;
        };
    };

};

} // namespace slk
