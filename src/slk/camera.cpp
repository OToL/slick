#include "camera.hpp"
#include <slk/math/graphics.hpp>
#include <slk/math/utils.hpp>

using namespace slk::literals;

namespace slk {

Camera::Camera(ETag<EProjection::ORTHO>, f32 near, f32 far, f32 left, f32 right, f32 top, f32 bottom, f32 offset) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(EProjection::ORTHO),
    m_left(left),
    m_right(right),
    m_top(top),
    m_bottom(bottom),
    m_offset(offset)
{
}

Camera::Camera(ETag<EProjection::PERSPECTIVE>, f32 near, f32 far, f32 fovy, f32 aspect) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(EProjection::PERSPECTIVE),
    m_fovy(fovy),
    m_aspect(aspect)
{
}


Camera::Camera() :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(0.1f),
    m_far(10000.f),
    m_projection(EProjection::PERSPECTIVE),
    m_fovy(45.0_deg),
    m_aspect(1.77777779) // 16:9
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

Vector3f Camera::right() const {
    return m_orientation.xAxis();
}

Vector3f Camera::up() const {
    return m_orientation.yAxis();
}

Vector3f Camera::forward() const {
    return m_orientation.zAxis();
}

// When looking straight up/down (|forward.y| ~ 1) yaw & roll rotate around the same axis (gimbal lock),
// only their sum is defined: roll is then 0 and yaw holds the whole rotation
static constexpr f32 GIMBAL_LOCK_THRESHOLD = 0.9999f;

f32 Camera::yaw() const {
    Vector3f const f = forward();
    if (std::abs(f.m_y) < GIMBAL_LOCK_THRESHOLD)
        return std::atan2(f.m_x, f.m_z);

    Vector3f const r = right();
    return std::atan2(-r.m_z, r.m_x);
}

f32 Camera::roll() const {
    if (std::abs(forward().m_y) < GIMBAL_LOCK_THRESHOLD)
        return std::atan2(right().m_y, up().m_y);

    return 0.f;
}

f32 Camera::pitch() const {
    // orientation is Ry(yaw) * Rx(pitch) i.e. forward = (sin(yaw)cos(pitch), -sin(pitch), cos(yaw)cos(pitch))
    Vector3f const f = forward();
    return std::atan2(-f.m_y, std::sqrt(f.m_x * f.m_x + f.m_z * f.m_z));
}

void Camera::translate(Vector3f const& vec) {
    m_pos += right() * vec.m_x + up() * vec.m_y + forward() * vec.m_z;
}

void Camera::pan(float delta_x, float delta_y) {
    m_pos += right() * delta_x + up() * delta_y;
}

Matrix4f Camera::viewMatrix() const {
    Matrix4f view_mat;
    view_mat.setTranslation(m_pos);
    view_mat.setRotation(m_orientation);
    view_mat.inverse();

    return view_mat;
}

void Camera::setPerspectiveProjection(f32 near, f32 far, f32 fovy, f32 aspect)
{
    m_near = near;
    m_far = far;
    m_aspect = aspect;
    m_fovy = fovy;
    m_projection = EProjection::PERSPECTIVE;
}

void Camera::setOrthoProjection(f32 near, f32 far, f32 left, f32 right, f32 top, f32 bottom, f32 offset)
{
    m_near = near;
    m_far = far;
    m_left = left;
    m_right = right;
    m_top = top;
    m_bottom = bottom;
    m_offset = offset;
    m_projection = EProjection::ORTHO;
}

Matrix4f Camera::projectionMatrix(EHandedness hand, bool homogeneous_ndc) const {

    if (m_projection == EProjection::ORTHO) {
        return makeOrthoProjectionMatrix(m_left, m_right, m_bottom, m_top, m_near, m_far, m_offset, homogeneous_ndc, hand);
    }
    else {
        return makePerspectiveProjectionMatrix(m_fovy, m_aspect, m_near, m_far, homogeneous_ndc, hand);
    }
}

Ray3f Camera::rayFromNdc(Vector2f ndc) {
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
        f32 const half_height_z1  = std::tan(0.5f * m_fovy);
        return Ray3f{
            .m_pos = m_pos,
            .m_dir = m_orientation * Vector3f{
                ndc.m_x * half_height_z1 * m_aspect,
                ndc.m_y * half_height_z1,
                1.f
            }
        };
    }
    else {
        Ray3f ray = {};
        return ray;
    }
}

} // namespace slk
