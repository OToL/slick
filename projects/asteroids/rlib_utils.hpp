#pragma once

#include "fwd.hpp"
#include <raylib/raylib.h>

namespace slk {
    struct ColorU32;
}
struct WindowCtx;

Color to_rcolor(slk::ColorU32 const& color);
Vector2 to_rvec2(slk::Vector2f const& vec); 
Vector2 to_rvec2(WindowCtx const& wnd_ctx, slk::Vector2f const& vec); 

