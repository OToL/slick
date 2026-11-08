#pragma once

#include <slk/math/vector3.hpp>
#include <slk/math/ray3.hpp>
#include <slk/math/math.hpp>
#include <slk/math/matrix3.hpp>
#include <slk/math/fwd.hpp>

namespace slk {

// TODO: Use a QTransform or Transform struct
//       i.e. compact representation of translation/orientation/scale
struct [[nodiscard]] Camera {

    Camera();
    Camera(OrthoProjectionParams const& params, f32 near, f32 far);
    Camera(PerspectiveProjectionParams const& params, f32 near, f32 far);

    // axis
    Vector3f rightAxis() const;
    Vector3f upAxis() const;
    Vector3f forwardAxis() const;

    // Euler angles of the orientation, decomposed as Ry(yaw) * Rx(pitch) * Rz(roll)
    // Note: same sign as the rotation matrices, rotate() applies -delta_yaw & -delta_pitch
    f32 yaw() const;
    // > 0 when looking down
    f32 pitch() const;
    // always 0 unless m_orientation is set directly, rotate() never rolls
    f32 roll() const;

    // movements
    void rotate(f32 delta_yaw, f32 delta_pitch);
    void translate(Vector3f vec);
    void pan(f32 delta_x, f32 delta_y);

    // view/proj
    void setLookAt(Vector3f eye, Vector3f target);
    Matrix4f viewMatrix() const;

    // utilities
    Ray3f rayFromNdc(Vector2f ndc) const;

    // projections
    void setPerspectiveProjection(PerspectiveProjectionParams const& params, f32 near, f32 far);
    void setOrthoProjection(OrthoProjectionParams const& params, f32 near, f32 far);
    Matrix4f projectionMatrix(EHandedness hand, bool homogeneous_ndc) const;

    Matrix3f m_orientation;
    Vector3f m_pos;
    f32 m_near;
    f32 m_far;

    EProjection m_projection;
    union {
        PerspectiveProjectionParams m_perspective;
        OrthoProjectionParams m_ortho;
    };

};

} // namespace slk
