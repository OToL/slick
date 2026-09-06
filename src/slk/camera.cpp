#include "camera.hpp"
#include <slk/math/graphics.hpp>
#include <slk/math/utils.hpp>

using namespace slk::literals;

namespace slk {

Camera::Camera(EnumTag<Projection::ORTHO>, f32 near, f32 far, f32 left, f32 right, f32 top, f32 bottom, f32 offset) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(Projection::ORTHO),
    m_left(left),
    m_right(right),
    m_top(top),
    m_bottom(bottom),
    m_offset(offset)
{
}

Camera::Camera(EnumTag<Projection::PERSPECTIVE>, f32 near, f32 far, f32 fovy, f32 aspect) :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(near),
    m_far(far),
    m_projection(Projection::PERSPECTIVE),
    m_fovy(fovy),
    m_aspect(aspect)
{
}


Camera::Camera() :
    m_orientation(Matrix3f::IDENTITY),
    m_pos(0.f, 0.f, 0.f),
    m_near(0.1f),
    m_far(10000.f),
    m_projection(Projection::PERSPECTIVE),
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
    roty.setRotationY(delta_yaw);
    rotx.setRotationX(delta_pitch);
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

void Camera::translate(Vector3f const& vec) {
    m_pos += right() * vec.m_x + up() * vec.m_y + forward() * vec.m_z;
}

void Camera::pan(float delta_x, float delta_y) {
    m_pos += right() * delta_x + Vector3f{0.f, delta_y, 0.f};
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
    m_projection = Projection::PERSPECTIVE;
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
    m_projection = Projection::ORTHO;
}

Matrix4f Camera::projectionMatrix(Handedness hand, bool homogeneous_ndc) const {

    if (m_projection == Projection::ORTHO) {
        return makeOrthoProjectionMatrix(m_left, m_right, m_bottom, m_top, m_near, m_far, m_offset, homogeneous_ndc, hand);
    }
    else {
        return makePerspectiveProjectionMatrix(m_fovy, m_aspect, m_near, m_far, homogeneous_ndc, hand);
    }
}


} // namespace slk
