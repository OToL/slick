#include "conversion.hpp"

#include <core/color.hpp>
#include <core/math/vector2.hpp>
#include <core/math/vector3.hpp>
#include <core/math/vector4.hpp>

::Color slk::to_rcolor(slk::ColorU32 const& color) {
    return {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

::Vector2 slk::to_rvec2(slk::Vector2f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y
    };
}

::Vector3 slk::to_rvec3(slk::Vector3f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y,
        .z = vec.z,
    };
}

::Vector4 slk::to_rvec4(slk::Vector4f const& vec) {
    return {
        .x = vec.x,
        .y = vec.y,
        .z = vec.z,
        .w = vec.w,
    };
}

