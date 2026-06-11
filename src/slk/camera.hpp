#pragma once

#include <slk/math/vector3.hpp>
#include <slk/math/matrix3.hpp>
#include <slk/math/fwd.hpp>

namespace slk {

struct Camera {

    Vector3f get_right() const;
    Vector3f get_up() const;
    Vector3f get_forward() const;

    // Vector3f get_target() const; // eye_pos + forward * orbit_distance
    //
    // void orbit(float delta_yaw, float delta_pitch);
    // void zoom(float delta);

    void rotate(f32 delta_yaw, f32 delta_pitch);
    void translate(Vector3f const& vec);
    void pan(f32 delta_x, f32 delta_y);

    void set_look_at(Vector3f const& eye, Vector3f const& target);
    void get_view_matrix(Matrix4f& view_mat) const;

    Matrix3f orientation;
    Vector3f pos;
};

} // namespace slk
