#include "rlib_utils.hpp"
#include "graphics.hpp"

#include <slk/math/vector2.hpp>
#include <slk/color.hpp>

Color to_rcolor(slk::ColorU32 const& color) {
    return {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

Vector2 to_rvec2(slk::Vector2f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y
    };
}

Vector2 to_rvec2(WindowCtx const& wnd_ctx, slk::Vector2f const& vec) 
{
    return {
        .x = vec.x,
        .y = wnd_ctx.height - vec.y
    };

}
