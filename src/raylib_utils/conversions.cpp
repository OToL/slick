#include "conversions.hpp"

#include <slk/color.hpp>
#include <slk/math/vector2.hpp>
#include <slk/math/vector3.hpp>
#include <slk/math/vector4.hpp>

::Color slk::convert_to_rcolor(slk::ColorU32 const& color) {
    return {
        .r = color.m_red,
        .g = color.m_green,
        .b = color.m_blue,
        .a = color.m_alpha
    };
}

::Vector2 slk::convert_to_rvec2(slk::Vector2f const& vec) {
    return {
        .x = vec.m_x,
        .y = vec.m_y
    };
}

::Vector3 slk::convert_to_rvec3(slk::Vector3f const& vec) {
    return {
        .x = vec.m_x,
        .y = vec.m_y,
        .z = vec.m_z,
    };
}

::Vector4 slk::convert_to_rvec4(slk::Vector4f const& vec) {
    return {
        .x = vec.m_x,
        .y = vec.m_y,
        .z = vec.m_z,
        .w = vec.m_w,
    };
}

