#include "cannon.hpp"

#include <slk/math/matrix2.hpp>

#include <algorithm>

Cannon::Cannon(slk::Vector2f pos, slk::f32 width, float height, float max_rot_rad, float shoot_freq_ms, float proj_speed)
    : _rot_curr_rad(0.f)
    , _shoot_cooldown(0.f)
    , _pos(pos)
    , _shoot_freq_ms(shoot_freq_ms)
    , _rot_max_rad(max_rot_rad)
    , _width(width)
    , _height(height)
    , _proj_speed(proj_speed) { }

void Cannon::update(slk::f32 delta_ms) {
    if (_shoot_cooldown > 0.f) {
        _shoot_cooldown -= delta_ms;
    }
}

std::optional<Projectile> Cannon::shoot() {
    if (_shoot_cooldown <= 0.f) {
        _shoot_cooldown = _shoot_freq_ms;

        const slk::Vector2f tip_dir = getAimingDir();
        const slk::Vector2f tip_scaled_dir = tip_dir * getHeight();
        const slk::Vector2f projectile_pos = _pos + tip_scaled_dir;

        return Projectile{
            .velocity = tip_dir * _proj_speed,
            .prev_pos = projectile_pos,
            .curr_pos = projectile_pos,
        };
    }

    return std::nullopt;
}

void Cannon::reset() {
    _rot_curr_rad = 0;
    _shoot_cooldown = 0.f;
}

void Cannon::turn(slk::f32 rot_rad) {
    _rot_curr_rad = std::clamp(_rot_curr_rad + rot_rad, -_rot_max_rad, _rot_max_rad);
}

slk::Vector2f Cannon::getAimingDir() const {
    const slk::Matrix2f xform = slk::Matrix2f::make_rotation(_rot_curr_rad);
    return xform * slk::Vector2f::unitY();
}
