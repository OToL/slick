#pragma once

#include <slk/core.hpp>
#include <slk/features.hpp>

import std;

//
// Polymonial based curves (https://www.youtube.com/@JeromeEippers/playlists).
//
// Curve: Mathematical function (e.g. polynomial) producing an interpolation or approximation between points based on time  
// Interpolation: The curve is going through the points being interpolated
// Approximation: The curve does its best to fit points but does not go through them i.e. they are more "guiding points"
// Spline: Special type of curve formed by connecting multiple curves (aka segment or section) together in a smooth way
//

namespace slk {

//
// Bezier
//
// Uses N control points to perform a polynomial-based interpolation (quadratic, cubic, etc.) from the 1st and last one. 
// It works by recursively interpolating the distance between the N control points.
//
// When working with a time interval other than [0, 1[, we cannot simply linearly map our new interval to it.
// Instead, the bezier root must be computed and used to properly map values from one interval to the other (e.g. [0, 25[ --> [0, 1[).
//
// degree = polynomial degree e.g.
//  - 2 (quadratic) : 3 control points
//  - 3 (cubic) : 4 control points
//
//  A Bezier spline is a combination of Bezier curves created by adding a multiple of `degree` (e.g. quadratic=2, cubic=3, etc.) control points
//

// Canonical Bezier curve of degree `ctrl_points.size() - 1` 
template<typename T> T evaluateBezierCurve(std::span<T const> ctrl_points, slk::f32 time);
template<typename T> T evaluateBezierCurve(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);

// Degree-2 polynomial Bezier curve i.e. 3 control points
template<typename T> T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, f32 time);
template<typename T> T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, std::span<f32 const, 3> ctrl_weights, f32 time);

// Degree-3 polynomial Bezier curve i.e. 4 control points
template<typename T> T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, f32 time);
template<typename T> T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, std::span<f32 const, 4> weights, f32 time);

struct BezierSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

BezierSplineInfo computeBezierSplineInfoFromSections(u32 section_cnt);
BezierSplineInfo computeBezierSplineInfoFromCtrlPoints(u32 ctrl_point_cnt);

template<typename T> T evaluateBezierSpline(std::span<T const> ctrl_points, u32 degree, f32 time);
template<typename T> T evaluateBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);
template<typename T> T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, f32 time);
template<typename T> T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);
template<typename T> T evaluateCubicBezierSpline(std::span<T const> ctrl_points, f32 time);

//
// Hermite
//
// Performs a cubic (degree-3) polynomial interpolation between 2 points uing tangeants as a guide. 
// Input data can be either provided separately ([P0, .., Pn] [T0, ..., Tn]) or interleaved ([P0, T1, ... Pn, Tn]).
//
// Time interval [0, 1[ cannot be linearly mapped to others because it would stretch the curve.
// To maintain the curve shape, tangeants must be scaled by the target time interval e.g. [0, 5[ --> tangeants * 5
//
// A Hermite spline is a combination of Hermite curves created by 1 or more extra {control point, tangeant}.
//

template<typename T> T evaluateHermiteCurve(std::span<T const, 2> points, std::span<T const, 2>  tangeants, f32 time);
template<typename T> T evaluateHermiteCurve(std::span<T const, 4> data, f32 time);

struct HermiteSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 tangeant_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

HermiteSplineInfo computeHermiteSplineInfoFromPoints(u32 ctrl_point_cnt);
HermiteSplineInfo computeHermiteSplineInfoFromSections(u32 section_cnt);

template<typename T> T evaluateHermiteSpline(std::span<T const> points, std::span<T const>  tangeants, f32 time);
template<typename T> T evaluateHermiteSpline(std::span<T const> data, f32 time);

//
// Cardinal
//
// Uses 4 control points (P0, P1, P2 & P3) to perform a cubic (degree-3) polynomial interpolation between the 2 middle ones (P1 & P2)
// A Cardinal curve can be extended to its enpoints (i.e. spline) by either providing them twice (P0, P0, P1, P2, P3 & P3) or using its 'extended' version. 
// The tension parameter indicates how straight the interpolation is ...
//  - 0: loose i.e. polynomial-like
//  - 1: straight lines
//
// Since tangeants are derived from control points, scaling the time interval does not impact the curve computation/sampling.
// But this only work if the computation/sampling interval is uniform. Otherwise, the curve must be converted to Hermite and tangeants scaled accordingly.
//
// A Cardinal spline is a combination of Cardinal curves created by 1 or more extra control points.
//

template<typename T> T evaluateCardinalCurve(std::span<T const, 4> data, f32 tension, f32 time);

struct CardinalSplineInfo {
    slk::u32 ctrl_point_cnt = 0;
    slk::u32 section_cnt = 0;
    slk::u32 item_cnt = 0;
};

CardinalSplineInfo computeCardinalSplineInfoFromPoints(slk::u32 ctrl_point_cnt, b8 extended);
CardinalSplineInfo computeCardinalSplineInfoFromSections(slk::u32 section_cnt);

template<typename T> T evaluateCardinalSpline(std::span<T const> points, f32 tension, f32 time);
template<typename T> T evaluateCardinalSplineExtended(std::span<T const>  points, f32 tension, f32 time);

//
// Uniform BSpline
//
// Uses 4 control points (P0 .. P3) to perform a cubic approximation between P1 & P2. 
// It can also be extended by adding 1 or more control points.
// A Uniform BSpline can be extended to its endpoints (i.e. spline) by either providing them three times (P0, P0, P0, P1, P2, P3, P3 & P3) or using its 'extended' version.
// BSpline can be used to encode complex functions (e.g. mapping raw key frames) to its parameters.
//

struct BSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

BSplineInfo computeBSplineInfoFromPoints(slk::u32 ctrl_point_cnt, b8 extended);
BSplineInfo computeBSplineInfoFromSections(slk::u32 section_cnt);

template<typename T> T evaluateBSplineSection(std::span<T const, 4> ctrl_points, f32 time);
template<typename T> T evaluateBSpline(std::span<T const> ctrl_points, f32 time);
template<typename T> T evaluateBSplineExtended(std::span<T const> ctrl_points, f32 time);

// 
// Splines conversion
//
template<typename T> void convertCardinalSplineToHermite(std::span<T const> points, std::span<T>  hermite_points, std::span<T> const hermite_tangeants, f32 tension, b8 extended);
template<typename T> void convertCardinalSplineToHermite(std::span<T const> points, std::span<T>  hermite_data, f32 tension, b8 extended);
template<typename T> void convertHermiteSplineToBezier(std::span<T const> points, std::span<T const>  tangeants, std::span<T> bezier_data);
template<typename T> void convertHermiteSplineToBezier(std::span<T const> data, std::span<T>  bezier_data);

} // namespace slk

#include sb_inline_hpp("curves.inl")

