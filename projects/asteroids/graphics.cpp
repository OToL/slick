#include "graphics.hpp"

#include <slk/color.hpp>
#include <slk/math/vector2.hpp>
#include <slk/math/matrix2.hpp>
#include <slk/math/aabb2.hpp>

#include <bgfx_utils/debugdraw/debugdraw.h>

#include <iterator>

namespace {

bx::Vec3 to_bxvec3(slk::Vector2f const& vec) {
    return {vec.m_x, vec.m_y, 0.f};
}

} // namespace

// Debug draw lines are 1 pixel wide, so thick lines are drawn as a quad (2 triangles)
void draw_line(DebugDrawEncoder& dde, slk::Vector2f const& start, slk::Vector2f const& end, slk::ColorU32 const& color, slk::f32 thickness) {
    slk::Vector2f const dir = end - start;
    if (dir.length2() == 0.f) {
        return;
    }

    slk::Vector2f const dir_norm = dir.normalized();
    slk::Vector2f const offset = slk::Vector2f{-dir_norm.m_y, dir_norm.m_x} * (thickness * 0.5f);

    dde.setColor(color.m_abgr);
    dde.draw(bx::Triangle{to_bxvec3(start + offset), to_bxvec3(start - offset), to_bxvec3(end + offset)});
    dde.draw(bx::Triangle{to_bxvec3(start - offset), to_bxvec3(end - offset), to_bxvec3(end + offset)});
}

void draw_aabb(DebugDrawEncoder& dde, slk::AABB2f const& aabb, slk::Vector2f const& pos, slk::ColorU32 const& color) {
    slk::Vector2f const min = aabb.m_min + pos;
    slk::Vector2f const max = aabb.m_max + pos;

    // horizontal edges are extended to fill the corners
    slk::f32 const half_thickness = DEFAULT_LINE_THICKNESS * 0.5f;

    draw_line(dde, {min.m_x - half_thickness, min.m_y}, {max.m_x + half_thickness, min.m_y}, color);
    draw_line(dde, {min.m_x - half_thickness, max.m_y}, {max.m_x + half_thickness, max.m_y}, color);
    draw_line(dde, {min.m_x, min.m_y}, {min.m_x, max.m_y}, color);
    draw_line(dde, {max.m_x, min.m_y}, {max.m_x, max.m_y}, color);
}

void draw_triangle(DebugDrawEncoder& dde, slk::Vector2f const& pos, slk::f32 rot_rad, slk::f32 width, slk::f32 height, slk::ColorU32 const& color)
{
    slk::Vector2f vertices[] = {
        {0, 1},
        {1, 0},
        {-1, 0},
    };

    // rotation
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

    for (size_t i = 0 ; i < std::size(vertices) ; ++i) {
        draw_line(dde, vertices[i], vertices[(i + 1) % std::size(vertices)], color);
    }
}
