#pragma once

#include <slk/math/vector2.hpp>
#include <slk/math/aabb2.hpp>

struct Ennemy {
    slk::Vector2f prev_position;
    slk::Vector2f position;
    slk::Vector2f velocity;
    slk::AABB2f extents;
};
