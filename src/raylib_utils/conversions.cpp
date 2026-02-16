#include "conversions.hpp"

#include <slk/color.hpp>
#include <slk/math/vector2.hpp>
#include <slk/math/vector3.hpp>
#include <slk/math/vector4.hpp>

::Color slk::convert_to_rcolor(slk::ColorU32 const& color) {
    return {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

::Vector2 slk::convert_to_rvec2(slk::Vector2f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y
    };
}

::Vector3 slk::convert_to_rvec3(slk::Vector3f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y,
        .z = vec.z,
    };
}

::Vector4 slk::convert_to_rvec4(slk::Vector4f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y,
        .z = vec.z,
        .w = vec.w,
    };
}

