#include <slk/core.hpp>

#include "curves.hpp"

#include sb_inline_cpp("curves.inl")

namespace slk {

BezierSplineInfo computeBezierSplineInfoFromCtrlPoints(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const item_cnt = 4 + 3 * (section_cnt - 1);
    return {
        .ctrl_point_cnt = item_cnt,
        .section_cnt = section_cnt,
        .item_cnt = item_cnt,
    };
};

BezierSplineInfo computeBezierSplineInfoFromSections(u32 ctrl_point_cnt) {
    if (ctrl_point_cnt < 4)
        return {};

    return {
        .ctrl_point_cnt = ctrl_point_cnt,
        .section_cnt = 1 + (ctrl_point_cnt - 4) / 3,
        .item_cnt = ctrl_point_cnt,
    };
};

HermiteSplineInfo computeHermiteSplineInfoFromPoints(u32 ctrl_point_cnt) {
    if (ctrl_point_cnt < 2)
        return {};

    return {.ctrl_point_cnt = ctrl_point_cnt, .tangeant_cnt = ctrl_point_cnt, .section_cnt = ctrl_point_cnt - 1, .item_cnt = ctrl_point_cnt << 1};
};

HermiteSplineInfo computeHermiteSplineInfoFromSections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = section_cnt + 1;

    return {.ctrl_point_cnt = point_cnt, .tangeant_cnt = point_cnt, .section_cnt = section_cnt, .item_cnt = point_cnt << 1};
};

CardinalSplineInfo computeCardinalSplineInfoFromPoints(u32 point_cnt, b8 extended) {
    if (point_cnt < 4)
        return {};

    return {.ctrl_point_cnt = point_cnt, .section_cnt = point_cnt - 3 + extended * 2, .item_cnt = point_cnt};
};

CardinalSplineInfo computeCardinalSplineInfoFromSections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = 4 + (section_cnt - 1);

    return {.ctrl_point_cnt = point_cnt, .section_cnt = section_cnt, .item_cnt = point_cnt};
};

BSplineInfo computeBSplineInfoFromPoints(slk::u32 ctrl_point_cnt, b8 extended) {
    if (ctrl_point_cnt < 4)
        return {};

    return {.ctrl_point_cnt = ctrl_point_cnt, .section_cnt = ctrl_point_cnt - 3 + extended * 4, .item_cnt = ctrl_point_cnt};
}

BSplineInfo computeBSplineInfoFromSections(slk::u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = 4 + (section_cnt - 1);

    return {.ctrl_point_cnt = point_cnt, .section_cnt = section_cnt, .item_cnt = point_cnt};
}

} // namespace slk

#if sb_feature_disabled(INCLUDE_INLINE)

#include "vector2.hpp"

import std;

#define sb_define_bezier_curve(type)                                                                                                                 \
    namespace slk {                                                                                                                                  \
        template type evaluateBezierCurve<type>(std::span<type const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);                     \
        template type evaluateBezierCurve<type>(std::span<type const> ctrl_points, f32 time);                                                        \
        template type evaluateQuadraticBezierCurve<type>(std::span<type const, 3> ctrl_points, std::span<f32 const, 3> weights, f32 time);           \
        template type evaluateQuadraticBezierCurve<type>(std::span<type const, 3> ctrl_points, f32 time);                                            \
        template type evaluateCubicBezierCurve<type>(std::span<type const, 4> ctrl_points, std::span<f32 const, 4> weights, f32 time);               \
        template type evaluateCubicBezierCurve<type>(std::span<type const, 4> ctrl_points, f32 time);                                                \
        template type evaluateBezierSpline<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);             \
        template type evaluateBezierSpline<type>(std::span<type const> ctrl_points, u32 degree, f32 time);                                           \
        template type evaluateQuadraticBezierSpline<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);                \
        template type evaluateQuadraticBezierSpline<type>(std::span<type const> ctrl_points, f32 time);                                              \
        template type evaluateCubicBezierSpline<type>(std::span<type const> ctrl_points, f32 time);                                                  \
    }

#define sb_define_hermite_curve(type)                                                                                                                \
    namespace slk {                                                                                                                                  \
        template type evaluateHermiteCurve<type>(std::span<type const, 4> data, f32 time);                                                           \
        template type evaluateHermiteCurve<type>(std::span<type const, 2> points, std::span<type const, 2> tangeants, f32 time);                     \
        template type evaluateHermiteSpline<type>(std::span<type const> data, f32 time);                                                             \
        template type evaluateHermiteSpline<type>(std::span<type const> points, std::span<type const> tangeants, f32 time);                          \
    }

#define sb_define_cardinal_curve(type)                                                                                                               \
    namespace slk {                                                                                                                                  \
        template type evaluateCardinalCurve(std::span<type const, 4> data, f32 tension, f32 time);                                                   \
        template type evaluateCardinalSpline(std::span<type const> points, f32 tension, f32 time);                                                   \
        template type evaluateCardinalSplineExtended(std::span<type const> points, f32 tension, f32 time);                                           \
    }

#define sb_define_bspline(type)                                                                                                                      \
    namespace slk {                                                                                                                                  \
        template type evaluateBSplineSection<type>(std::span<type const, 4> ctrl_points, f32 time);                                                  \
        template type evaluateBSpline<type>(std::span<type const> ctrl_points, f32 time);                                                            \
        template type evaluateBSplineExtended<type>(std::span<type const> ctrl_points, f32 time);                                                    \
    }

#define sb_define_spline_conversions(type)                                                                                                           \
    namespace slk {                                                                                                                                  \
        template void convertCardinalSplineToHermite(std::span<type const> points, std::span<type> hermite_data, f32 tension, b8 extended);          \
        template void convertCardinalSplineToHermite(std::span<type const> points, std::span<type> hermite_points,                                   \
                                                     std::span<type> const hermite_tangeants, f32 tension, b8 extended);                             \
        template void convertHermiteSplineToBezier(std::span<type const> data, std::span<type> bezier_data);                                         \
        template void convertHermiteSplineToBezier(std::span<type const> points, std::span<type const> tangeants, std::span<type> bezier_data);      \
    }

sb_define_bezier_curve(Vector4f) 
sb_define_bezier_curve(Vector3f) 
sb_define_bezier_curve(Vector2f) 
sb_define_bezier_curve(f32)

sb_define_hermite_curve(Vector4f) 
sb_define_hermite_curve(Vector3f) 
sb_define_hermite_curve(Vector2f) 
sb_define_hermite_curve(f32)

sb_define_cardinal_curve(Vector4f)
sb_define_cardinal_curve(Vector3f)
sb_define_cardinal_curve(Vector2f) 
sb_define_cardinal_curve(f32)

sb_define_bspline(Vector4f);
sb_define_bspline(Vector3f);
sb_define_bspline(Vector2f);
sb_define_bspline(f32);

sb_define_spline_conversions(Vector4f)
sb_define_spline_conversions(Vector3f)
sb_define_spline_conversions(Vector2f)
sb_define_spline_conversions(f32)

#endif
