#pragma once

#include <slk/core.hpp>

#include "fwd.hpp"

namespace slk {
    struct ColorU32;
}

struct DebugDrawEncoder;

constexpr slk::f32 DEFAULT_LINE_THICKNESS = 2.f;

// All coordinates are in pixels, origin bottom-left, y up
void draw_line(DebugDrawEncoder& dde, slk::Vector2f const& start, slk::Vector2f const& end, slk::ColorU32 const& color, slk::f32 thickness = DEFAULT_LINE_THICKNESS);
void draw_aabb(DebugDrawEncoder& dde, slk::AABB2f const& aabb, slk::Vector2f const& pos, slk::ColorU32 const& color);
void draw_triangle(DebugDrawEncoder& dde, slk::Vector2f const& pos, slk::f32 rot_rad, slk::f32 width, slk::f32 height, slk::ColorU32 const& color);
