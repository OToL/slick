#include "camera.hpp"
#include <slk/math/graphics.hpp>
#include <slk/math/utils.hpp>

#include <cassert>

using namespace slk::literals;

namespace slk {

Camera::Camera(OrthoProjectionParams const& params, f32 near, f32 far) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(EProjection::ORTHO),
    m_ortho(params)
{
}

Camera::Camera(PerspectiveProjectionParams const& params, f32 near, f32 far) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(EProjection::PERSPECTIVE),
    m_perspective(params)
{
}

Camera::Camera() :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(0.1f),
    m_far(10000.f),
    m_projection(EProjection::PERSPECTIVE),
    m_perspective{
        .m_fovy = 45.0_deg,
        .m_aspect = 1.77777779 // 16:9
    }
{
}

void Camera::setLookAt(Vector3f eye, Vector3f target) {
    Matrix4f view = makeLookAtMatrix(eye, target, Vector3f::UNITY);
    ;
    view.inverse();

    m_orientation = view.rotation();
    m_pos = eye;
}

void Camera::rotate(f32 delta_yaw, f32 delta_pitch) {
    Matrix3f roty, rotx;
    roty.setRotationY(-delta_yaw);
    rotx.setRotationX(-delta_pitch);
    m_orientation = roty * m_orientation * rotx;
}

Vector3f Camera::rightAxis() const {
    return m_orientation.xAxis();
}

Vector3f Camera::upAxis() const {
    return m_orientation.yAxis();
}

Vector3f Camera::forwardAxis() const {
    return m_orientation.zAxis();
}

// When looking straight up/down (|forward.y| ~ 1) yaw & roll rotate around the same axis (gimbal lock),
// only their sum is defined: roll is then 0 and yaw holds the whole rotation
static constexpr f32 GIMBAL_LOCK_THRESHOLD = 0.9999f;

f32 Camera::yaw() const {
    Vector3f const f = forwardAxis();
    if (std::abs(f.m_y) < GIMBAL_LOCK_THRESHOLD)
        return std::atan2(f.m_x, f.m_z);

    Vector3f const r = rightAxis();
    return std::atan2(-r.m_z, r.m_x);
}

f32 Camera::roll() const {
    if (std::abs(forwardAxis().m_y) < GIMBAL_LOCK_THRESHOLD)
        return std::atan2(rightAxis().m_y, upAxis().m_y);

    return 0.f;
}

f32 Camera::pitch() const {
    // orientation is Ry(yaw) * Rx(pitch) i.e. forward = (sin(yaw)cos(pitch), -sin(pitch), cos(yaw)cos(pitch))
    Vector3f const f = forwardAxis();
    return std::atan2(-f.m_y, std::sqrt(f.m_x * f.m_x + f.m_z * f.m_z));
}

void Camera::translate(Vector3f vec) {
    m_pos += rightAxis() * vec.m_x + upAxis() * vec.m_y + forwardAxis() * vec.m_z;
}

void Camera::pan(float delta_x, float delta_y) {
    m_pos += rightAxis() * delta_x + upAxis() * delta_y;
}

Matrix4f Camera::viewMatrix() const {
    Matrix4f view_mat;
    view_mat.setTranslation(m_pos);
    view_mat.setRotation(m_orientation);
    view_mat.inverse();

    return view_mat;
}

void Camera::setPerspectiveProjection(PerspectiveProjectionParams const& params, f32 near, f32 far)
{
    m_near = near;
    m_far = far;

    m_projection = EProjection::PERSPECTIVE;
    m_perspective = params;
}

void Camera::setOrthoProjection(OrthoProjectionParams const& params, f32 near, f32 far)
{
    m_near = near;
    m_far = far;

    m_projection = EProjection::ORTHO;
    m_ortho = params;
}

Matrix4f Camera::projectionMatrix(EHandedness hand, bool homogeneous_ndc) const {

    if (m_projection == EProjection::ORTHO) {
        return makeOrthoProjectionMatrix(m_ortho, m_near, m_far, homogeneous_ndc, hand);
    }
    else {
        return makePerspectiveProjectionMatrix(m_perspective, m_near, m_far, homogeneous_ndc, hand);
    }
}

Ray3f Camera::rayFromNdc(Vector2f ndc) const {
    if (m_projection == EProjection::PERSPECTIVE)
    {
        // The camera frustrum is a pyramid and we can do our computations at any
        // camera `z` value as far as we keep consitent proportions. 
        // `z=1` is convenient because:
        //      tan(fovy/2)=half_height/1=half_height 
        //
        //           top edge of the view
        //          /|
        //         / |
        //        /  |  half_height
        //       /   |
        //      /    |
        // cam o-----+   ← z = 1
        //      \ θ  |
        //       \   |     θ = fovy / 2
        //        \  |
        //         \ |
        //          \|
        //           bottom edge
        f32 const half_height_z1  = std::tan(0.5f * m_perspective.m_fovy);
        return Ray3f{
            .m_pos = m_pos,
            .m_dir = (m_orientation * Vector3f{
                ndc.m_x * half_height_z1 * m_perspective.m_aspect,
                ndc.m_y * half_height_z1,
                1.f
            }).normalized()
        };
    }
    else {
        assert(false);
        Ray3f ray = {};
        return ray;
    }
}

} // namespace slk
