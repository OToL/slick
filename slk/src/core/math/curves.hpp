#pragma once

#include <core/core.hpp>

#include <span>

//
// Polymonial based curves.
//
// Curve/Section: Smallest granularity to perform an interpolation or approximation between points
// Interpolation: Curve is going through the 2 Points being interpolated
// Approximation: The curve does its best to fit points but does not go through them i.e. they are more "guiding points"
// Spline: Combination of 2 or more Curves/Sections back to back
//
// source: https://www.youtube.com/@JeromeEippers/playlists
//

namespace slk {

//
// Bezier
//
// Uses 4 control points to perform a polynomial-based interpolation (quadratic, cubic, etc.) from the 1st and last one. 
// It works by recursively interpolating the distance between the 4 control points.
// A Bezier curve can be upgraded to a spline by adding a multiple of 3 control points.
//
// When working with a time interval other than [0, 1[, we cannot simply linearly map our new interval to it.
// Instead, the bezier root must be computed and used to properly map values from one interval to the other (e.g. [0, 25[ --> [0, 1[).
//

struct BezierSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

BezierSplineInfo get_bezier_spline_info_from_points(u32 section_cnt);
BezierSplineInfo get_bezier_spline_info_from_sections(u32 ctrl_point_cnt);

// interleaved data: [P0, T0] ... [Pn, Tn] Vector3f compute_bezier_at(std::span<Vector3f const>  ctrl_points, f32 time);
template<typename T> T compute_bezier_curve_at(std::span<T const> ctrl_points, slk::f32 time);
template<typename T> T compute_bezier_curve_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);
template<typename T> T compute_bezier_quadratic_curve_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_quadratic_curve_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);
template<typename T> T compute_bezier_cubic_curve_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_cubic_curve_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);

// degree = polynomial degree e.g.
//  - 2 (quadratic) : 3 control points
//  - 3 (cubic) : 4 control points
template<typename T> T compute_bezier_spline_at(std::span<T const> ctrl_points, u32 degree, f32 time);
template<typename T> T compute_bezier_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);
template<typename T> T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_cubic_spline_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);

//
// Hermite
//
// Performs a polynomial-based interpolation (cubic, quadratic, etc.) between 2 points uing tangeants as a guide. 
// A Hermite curve can be upgraded to a spline by adding 1 or more extra point+tangeant.
// Input data can be either provided separately ([P0, .., Pn] [T0, ..., Tn]) or interleaved ([P0, T1, ... Pn, Tn]).
// A Bezier spline can be derived from an Hermite spline.
//
// Time interval [0, 1[ cannot be linearly mapped to others because it would stretch the curve.
// To maitain the curve shape, tangeants must be scaled by the target time interval e.g. [0, 5[ --> tangeants * 5
//

struct HermiteSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 tangeant_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

HermiteSplineInfo get_hermite_spline_info_from_points(u32 ctrl_point_cnt);
HermiteSplineInfo get_hermite_spline_info_from_sections(u32 section_cnt);

// separated data: [P0, ..., Pn] [T0, ..., Tn]
template<typename T> T compute_hermite_curve_at(std::span<T const> points, std::span<T const>  tangeants, f32 time);
template<typename T> T compute_hermite_curve_at(std::span<T const> data, f32 time);
template<typename T> T compute_hermite_spline_at(std::span<T const> points, std::span<T const>  tangeants, f32 time);
template<typename T> T compute_hermite_spline_at(std::span<T const> data, f32 time);

//
// Cardinal
//
// Uses 4 control points (P0, P1, P2 & P3) to perform a cubic interpolation between the 2 middle ones (P1 & P2)
// A Cardinal curve can be extended to its enpoints (i.e. spline) by either providing them twice (P0, P0, P1, P2, P3 & P3) or using its 'extended' version. 
// It can also be upgraded to a spline by adding 1 or more points.
// The tension parameter indicates how straight the interpolation is ...
//  - 0: loose i.e. polynomial-like
//  - 1: straight lines
//
// Since tangeants are derived from control points, scaling the time interval does not impact the curve computation/sampling.
// But this only work if the computation/sampling interval is uniform. Otherwise, the curve must be converted to Hermite and tangeants scaled accordingly.
//

struct CardinalSplineInfo {
    slk::u32 ctrl_point_cnt = 0;
    slk::u32 section_cnt = 0;
    slk::u32 item_cnt = 0;
};

CardinalSplineInfo get_cardinal_spline_info_from_points(slk::u32 ctrl_point_cnt, b8 extended);
CardinalSplineInfo get_cardinal_spline_info_from_sections(slk::u32 section_cnt);

template<typename T> T compute_cardinal_curve_at(std::span<T const> data, f32 tension, f32 time);
template<typename T> T compute_cardinal_spline_at(std::span<T const> points, f32 tension, f32 time);
template<typename T> T compute_cardinal_spline_extended_at(std::span<T const>  points, f32 tension, f32 time);

//
// Uniform BSpline
//
// Uses 4 control points (P0 .. P3) to perform a cubic approximation between P1 & P2. 
// A Uniform BSpline can be extended to its endpoints (i.e. spline) by either providing them three times (P0, P0, P0, P1, P2, P3, P3 & P3) or using its 'extended' version.
// It can also be extended by adding 1 or more control points.
// BSpline can be used to encode complex functions (e.g. mapping raw key frames) to its parameters.
//

struct BSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

BSplineInfo get_bspline_info_from_points(slk::u32 ctrl_point_cnt, b8 extended);
BSplineInfo get_bspline_info_from_sections(slk::u32 section_cnt);

template<typename T> T compute_bspline_cubic_section_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bspline_cubic_spline_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bspline_cubic_spline_extended_at(std::span<T const> ctrl_points, f32 time);

// 
// Splines conversion
//
template<typename T> void convert_cardinal_spline_to_hermite(std::span<T const> points, std::span<T>  hermite_points, std::span<T> const hermite_tangeants, f32 tension, b8 extended);
template<typename T> void convert_cardinal_spline_to_hermite(std::span<T const> points, std::span<T>  hermite_data, f32 tension, b8 extended);
template<typename T> void convert_hermite_spline_to_bezier(std::span<T const> points, std::span<T const>  tangeants, std::span<T> bezier_data);
template<typename T> void convert_hermite_spline_to_bezier(std::span<T const> data, std::span<T>  bezier_data);

} // namespace slk

