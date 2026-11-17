#include <slk/core.hpp>

#include "curves.hpp"

#include sb_inline_cpp("curves.inl")

#if sb_feature_disabled(INCLUDE_INLINE)

#include "vector2.hpp"

import std;

#define sb_define_bezier_curve(type)                                                                                                              \
    namespace slk {                                                                                                                               \
        template type evaluateBezierCurve<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);                       \
        template type evaluateBezierCurve<type>(std::span<type const> ctrl_points, f32 time);                                                     \
        template type evaluateQuadraticBezierCurve<type>(std::span<type const, 3> ctrl_points, std::span<f32 const, 3> weights, f32 time);        \
        template type evaluateQuadraticBezierCurve<type>(std::span<type const, 3> ctrl_points, f32 time);                                         \
        template type evaluateCubicBezierCurve<type>(std::span<type const, 4> ctrl_points, std::span<f32 const, 4> weights, f32 time);            \
        template type evaluateCubicBezierCurve<type>(std::span<type const, 4> ctrl_points, f32 time);                                             \
        template type evaluateBezierSpline<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);          \
        template type evaluateBezierSpline<type>(std::span<type const> ctrl_points, u32 degree, f32 time);                                        \
        template type evaluateQuadraticBezierSpline<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);             \
        template type evaluateQuadraticBezierSpline<type>(std::span<type const> ctrl_points, f32 time);                                           \
        template type evaluateCubicBezierSpline<type>(std::span<type const> ctrl_points, f32 time);                                               \
    }

#define sb_define_hermite_curve(type)                                                                                                             \
    namespace slk {                                                                                                                               \
        template type evaluateHermiteCurve<type>(std::span<HermiteKey<type> const, 2> keys, f32 time);                                            \
        template type evaluateHermiteCurve<type>(std::span<type const, 2> points, std::span<type const, 2> tangents, f32 time);                   \
        template type evaluateHermiteSpline<type>(std::span<HermiteKey<type> const> keys, f32 time);                                              \
        template type evaluateHermiteSpline<type>(std::span<type const> points, std::span<type const> tangents, f32 time);                        \
    }

#define sb_define_cardinal_curve(type)                                                                                                            \
    namespace slk {                                                                                                                               \
        template type evaluateCardinalCurve<type>(std::span<type const, 4> points, f32 tension, f32 time);                                        \
        template type evaluateCardinalSpline<type>(std::span<type const> points, f32 tension, ESplineEnds ends, f32 time);                        \
    }

#define sb_define_bspline(type)                                                                                                                   \
    namespace slk {                                                                                                                               \
        template type evaluateBSplineCurve<type>(std::span<type const, 4> ctrl_points, f32 time);                                                 \
        template type evaluateBSpline<type>(std::span<type const> ctrl_points, ESplineEnds ends, f32 time);                                       \
    }

#define sb_define_spline_conversions(type)                                                                                                        \
    namespace slk {                                                                                                                               \
        template void convertCardinalSplineToHermite<type>(std::span<type const> points, std::span<HermiteKey<type>> hermite_keys, f32 tension,   \
                                                           ESplineEnds ends);                                                                     \
        template void convertCardinalSplineToHermite<type>(std::span<type const> points, std::span<type> hermite_points,                          \
                                                           std::span<type> hermite_tangents, f32 tension, ESplineEnds ends);                      \
        template void convertHermiteSplineToBezier<type>(std::span<HermiteKey<type> const> keys, std::span<type> bezier_points);                  \
        template void convertHermiteSplineToBezier<type>(std::span<type const> points, std::span<type const> tangents, std::span<type> bezier_points);\
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

sb_define_bspline(Vector4f)
sb_define_bspline(Vector3f)
sb_define_bspline(Vector2f)
sb_define_bspline(f32)

sb_define_spline_conversions(Vector4f)
sb_define_spline_conversions(Vector3f)
sb_define_spline_conversions(Vector2f)
sb_define_spline_conversions(f32)

#endif
