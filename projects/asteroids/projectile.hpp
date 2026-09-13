#pragma once

#include <slk/math/vector2.hpp>

struct Projectile {
    slk::Vector2f velocity;
    slk::Vector2f prev_pos;
    slk::Vector2f curr_pos;
};

