#pragma once

#include <slk/math/fwd.hpp>

#include <raylib/raylib.h>

namespace slk {

struct ColorU32;

::Color convert_to_rcolor(slk::ColorU32 const& color);
::Vector2 convert_to_rvec2(slk::Vector2f const& vec); 
::Vector3 convert_to_rvec3(slk::Vector3f const& vec); 
::Vector4 convert_to_rvec4(slk::Vector4f const& vec); 

}
