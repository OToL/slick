#pragma once

#include <core/core.hpp>
#include <core/math/vector2.hpp>
#include "projectile.hpp"

#include <optional>

class Cannon {
public:
    Cannon(slk::Vector2f pos, slk::f32 width, slk::f32 height, slk::f32 max_rot_rad, slk::f32 shoot_freq_ms, slk::f32 proj_speed);

    void reset();
    void update(slk::f32 delta_ms);
    void turn(slk::f32 rot_rad);
    std::optional<Projectile> shoot();

    slk::f32 getWidth() const {
        return _width;
    }

    slk::f32 getHeight() const {
        return _height;
    }

    slk::Vector2f getAimingDir() const;

    slk::f32 getRotation() const {
        return _rot_curr_rad;
    }

    slk::Vector2f getPos() const {
        return _pos;
    }

private:
    slk::f32 _rot_curr_rad;
    slk::f32 _shoot_cooldown;

    slk::Vector2f _pos;
    slk::f32 _shoot_freq_ms;
    slk::f32 _rot_max_rad;
    slk::f32 _width;
    slk::f32 _height;
    slk::f32 _proj_speed;
};
