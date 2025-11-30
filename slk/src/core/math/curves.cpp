#include <core/core.hpp>

#include "curves.hpp"
#include "curves.inl"

namespace slk {

BezierSplineInfo get_bezier_spline_info_from_points(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const item_cnt = 4 + 3 * (section_cnt - 1);
    return {
        .ctrl_point_cnt = item_cnt,
        .item_cnt = item_cnt,
        .section_cnt = section_cnt,
    };
};

BezierSplineInfo get_bezier_spline_info_from_sections(u32 ctrl_point_cnt) {
    if (ctrl_point_cnt < 4)
        return {};

    return {
        .ctrl_point_cnt = ctrl_point_cnt,
        .item_cnt = ctrl_point_cnt,
        .section_cnt = 1 + (ctrl_point_cnt - 4)/3,
    };
};

HermiteSplineInfo get_hermite_spline_info_from_points(u32 ctrl_point_cnt) {
    if (ctrl_point_cnt < 2)
        return {};

    return {
        .ctrl_point_cnt = ctrl_point_cnt,
        .tangeant_cnt = ctrl_point_cnt,
        .section_cnt = ctrl_point_cnt - 1,
        .item_cnt = ctrl_point_cnt << 1 
    };
};

HermiteSplineInfo get_hermite_spline_info_from_sections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = section_cnt + 1;

    return {
        .ctrl_point_cnt = point_cnt,
        .tangeant_cnt = point_cnt,
        .section_cnt = section_cnt,
        .item_cnt = point_cnt << 1
    };
};

CardinalSplineInfo get_cardinal_spline_info_from_points(u32 point_cnt, b8 extended) {
    if (point_cnt < 4)
        return {};

    return {
        .point_cnt = point_cnt,
        .section_cnt = point_cnt - 3 + extended*2,
        .item_cnt = point_cnt
    };
};

CardinalSplineInfo get_cardinal_spline_info_from_sections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = 4 + (section_cnt - 1);

    return {
        .point_cnt = point_cnt,
        .section_cnt = section_cnt,
        .item_cnt = point_cnt
    };
};

} // namespace slk

SB_DEFINE_CURVE_BEZIER(Vector3f)
SB_DEFINE_CURVE_BEZIER(f32)

SB_DEFINE_CURVE_HERMITE(Vector3f)
SB_DEFINE_CURVE_HERMITE(f32)

SB_DEFINE_CURVE_CARDINAL(Vector3f)
SB_DEFINE_CURVE_CARDINAL(f32)

SB_DEFINE_CURVE_CONVERSIONS(Vector3f)
SB_DEFINE_CURVE_CONVERSIONS(f32)


