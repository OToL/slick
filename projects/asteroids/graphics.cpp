#include "graphics.hpp"
#include "rlib_utils.hpp"

#include <raylib/raylib.h>
#include <raylib/raymath.h>

#include <slk/color.hpp>
#include <slk/math/vector2.hpp>
#include <slk/math/matrix2.hpp>
#include <slk/math/aabb2.hpp>

#include <iterator>

void draw_aabb(WindowCtx const& wnd_ctx, slk::AABB2f const& aabb, slk::Vector2f const& pos, slk::ColorU32 const& color) {

    const Rectangle rrect = {
        .x = aabb.m_min.m_x + pos.m_x,
        .y = wnd_ctx.height - aabb.m_max.m_y - pos.m_y,
        .width = aabb.m_max.m_x - aabb.m_min.m_x,
        .height = aabb.m_max.m_y - aabb.m_min.m_y
    };
    const ::Color rcolor = {
        .r = color.m_red,
        .g = color.m_green,
        .b = color.m_blue,
        .a = color.m_alpha
    };

    DrawRectangleLinesEx(rrect, 2.f, rcolor);
}

void draw_triangle(WindowCtx const& wnd_ctx, slk::Vector2f const& pos, slk::f32 rot_rad, slk::u32 width, slk::u32 height, slk::ColorU32 const& color)
{
    slk::Vector2f vertices[] = {
        {0, 1},
        {1, 0},
        {-1, 0},
    };

    // rotatiomathn
    slk::Matrix2f xform = slk::Matrix2f::makeRotation(rot_rad);
    for (auto& vert : vertices) {
        vert = xform * vert;
    }

    // scale
    vertices[0] *= height;
    vertices[1] *= width * 0.5f;
    vertices[2] *= width * 0.5f; 

    // translation
    for (auto& vert : vertices) {
        vert += pos;
    }

    // transform vertices
    Vector2 rvertices[4] = {};
    slk::u32 idx = 0;
    for (auto& rvert : rvertices) {

        rvert = {
            .x = vertices[idx].m_x,
            .y = wnd_ctx.height - vertices[idx].m_y
        };

        idx = (idx + 1)%std::size(vertices);
    }

    for (size_t i = 1 ; i < std::size(rvertices) ; ++i) {

        DrawLineEx(rvertices[i - 1], rvertices[i], 2, to_rcolor(color));
    }
}


