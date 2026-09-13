#include "rlib_utils.hpp"
#include "graphics.hpp"

#include <slk/math/vector2.hpp>
#include <slk/color.hpp>

Color to_rcolor(slk::ColorU32 const& color) {
    return {
        .r = color.m_red,
        .g = color.m_green,
        .b = color.m_blue,
        .a = color.m_alpha
    };
}

Vector2 to_rvec2(slk::Vector2f const& vec) {
    return {
        .x = vec.m_x,
        .y = vec.m_y
    };
}

Vector2 to_rvec2(WindowCtx const& wnd_ctx, slk::Vector2f const& vec) 
{
    return {
        .x = vec.m_x,
        .y = wnd_ctx.height - vec.m_y
    };

}
