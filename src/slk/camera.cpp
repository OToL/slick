#include "camera.hpp"
#include <slk/math/graphics.hpp>

void slk::Camera::set_look_at(Vector3f const& eye, Vector3f const& target) {
    slk::Matrix4f view = make_look_at(eye, target, slk::Vector3f::UNITY);;
    view.inverse();

    orientation = view.rotation();
    pos = eye;
}

void slk::Camera::rotate(f32 delta_yaw, f32 delta_pitch) {
    Matrix3f roty, rotx;
    roty.set_rotation_y(delta_yaw);
    rotx.set_rotation_x(delta_pitch);
    orientation = roty * orientation * rotx;
}

slk::Vector3f slk::Camera::get_right() const {
    return orientation.x_axis();
}

slk::Vector3f slk::Camera::get_up() const {
    return orientation.y_axis();
}

slk::Vector3f slk::Camera::get_forward() const {
    return orientation.z_axis();
}

void slk::Camera::translate(Vector3f const& vec) {
    pos += get_right() * vec.x + get_up() * vec.y + get_forward() * vec.z;
}

void slk::Camera::pan(float delta_x, float delta_y) {
    pos += get_right() * delta_x + Vector3f{0.f, delta_y, 0.f};
}

void slk::Camera::get_view_matrix(Matrix4f& view_mat) const {
    // Matrix3f const inv_rot = rotation.inversed();
    // view_mat.setRotation(inv_rot);
    // view_mat.setTranslation(inv_rot * (-pos));
    view_mat.set_translation(pos);
    view_mat.set_rotation(orientation);
    view_mat.inverse();
}
