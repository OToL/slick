#pragma once

#include <slk/core.hpp>

#include "fwd.hpp"
#include <raylib/raylib.h>

namespace slk {
    struct ColorU32;
}

struct WindowCtx {
    slk::u32 width;
    slk::u32 height;
};

void draw_aabb(WindowCtx const& wnd_ctx, slk::AABB2f const& aabb, slk::Vector2f const& pos, slk::ColorU32 const& color);
void draw_triangle(WindowCtx const& wnd_ctx, slk::Vector2f const& pos, slk::f32 rot_rad, slk::u32 width, unsigned height, slk::ColorU32 const& color);
