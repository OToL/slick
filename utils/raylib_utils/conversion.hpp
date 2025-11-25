#pragma once

#include <core/math/fwd.hpp>

#include <raylib/raylib.h>

namespace slk {

struct ColorU32;

::Color to_rcolor(slk::ColorU32 const& color);
::Vector2 to_rvec2(slk::Vector2f const& vec); 
::Vector3 to_rvec3(slk::Vector3f const& vec); 
::Vector4 to_rvec4(slk::Vector4f const& vec); 

}
