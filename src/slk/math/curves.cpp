#include <slk/core.hpp>

#include "curves.hpp"
#include "curves.inl"
#include "vector2.hpp"

namespace slk {

BezierSplineInfo get_bezier_spline_info(CurveFeatureSectionTag, u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const item_cnt = 4 + 3 * (section_cnt - 1);
    return {
        .ctrl_point_cnt = item_cnt,
        .section_cnt = section_cnt,
        .item_cnt = item_cnt,
    };
};

BezierSplineInfo get_bezier_spline_info(CurveFeaturePointTag, u32 ctrl_point_cnt) {
    if (ctrl_point_cnt < 4)
        return {};

    return {
        .ctrl_point_cnt = ctrl_point_cnt,
        .section_cnt = 1 + (ctrl_point_cnt - 4)/3,
        .item_cnt = ctrl_point_cnt,
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
        .ctrl_point_cnt = point_cnt,
        .section_cnt = point_cnt - 3 + extended*2,
        .item_cnt = point_cnt
    };
};

CardinalSplineInfo get_cardinal_spline_info_from_sections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = 4 + (section_cnt - 1);

    return {
        .ctrl_point_cnt = point_cnt,
        .section_cnt = section_cnt,
        .item_cnt = point_cnt
    };
};

BSplineInfo get_bspline_info_from_points(slk::u32 ctrl_point_cnt, b8 extended) {
    if (ctrl_point_cnt < 4)
        return {};

    return {
        .ctrl_point_cnt = ctrl_point_cnt,
        .section_cnt = ctrl_point_cnt - 3 + extended*4,
        .item_cnt = ctrl_point_cnt
    };

}

BSplineInfo get_bspline_info_from_sections(slk::u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = 4 + (section_cnt - 1);

    return {
        .ctrl_point_cnt = point_cnt,
        .section_cnt = section_cnt,
        .item_cnt = point_cnt
    };

}

} // namespace slk

SB_DEFINE_CURVE_BEZIER(Vector4f)
SB_DEFINE_CURVE_BEZIER(Vector3f)
SB_DEFINE_CURVE_BEZIER(Vector2f)
SB_DEFINE_CURVE_BEZIER(f32)

SB_DEFINE_CURVE_HERMITE(Vector4f)
SB_DEFINE_CURVE_HERMITE(Vector3f)
SB_DEFINE_CURVE_HERMITE(Vector2f)
SB_DEFINE_CURVE_HERMITE(f32)

SB_DEFINE_CURVE_CARDINAL(Vector4f)
SB_DEFINE_CURVE_CARDINAL(Vector3f)
SB_DEFINE_CURVE_CARDINAL(Vector2f)
SB_DEFINE_CURVE_CARDINAL(f32)

SB_DEFINE_CURVE_CONVERSIONS(Vector4f)
SB_DEFINE_CURVE_CONVERSIONS(Vector3f)
SB_DEFINE_CURVE_CONVERSIONS(Vector2f)
SB_DEFINE_CURVE_CONVERSIONS(f32)

SB_DEFINE_CURVE_BSPLINE(Vector4f);
SB_DEFINE_CURVE_BSPLINE(Vector3f);
SB_DEFINE_CURVE_BSPLINE(Vector2f);
SB_DEFINE_CURVE_BSPLINE(f32);

