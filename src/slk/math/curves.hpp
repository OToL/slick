#pragma once

#include <slk/core.hpp>
#include <slk/features.hpp>

#include <assert.h>

import std;

//
// Polymonial based curves (https://www.youtube.com/@JeromeEippers/playlists).
//
// Curve: Mathematical function (e.g. polynomial) producing an interpolation or approximation between points based on time
// Interpolation: The curve is going through the points being interpolated
// Approximation: The curve does its best to fit points but does not go through them i.e. they are more "guiding points"
// Spline: Special type of curve formed by connecting multiple curves (aka segment or section) together in a smooth way
//
// Time convention:
//  - `evaluate...Curve` functions take a time in [0, 1]
//  - `evaluate...Spline` functions take a time in [0, section count], clamped (see splitSplineTime)
//

namespace slk {

// How a spline handles its first / last control points
enum class ESplineEnds : u8 {
    OPEN,    // the spline only spans the inner control points (e.g. P1 -> Pn-2 for a cardinal spline)
    CLAMPED, // the spline reaches the first and last control points
};

struct [[nodiscard]] SplineLayout {
    u32 m_point_cnt = 0;   // control points (Hermite: number of {point, tangent} keys)
    u32 m_section_cnt = 0;
};

struct [[nodiscard]] SplineTime {
    u32 m_section = 0;
    f32 m_local_time = 0.f; // [0, 1] within m_section
};

// Splits a spline time into a section index and a local time, time is clamped to [0, section_cnt]
constexpr SplineTime splitSplineTime(f32 time, u32 section_cnt) {
    assert(section_cnt > 0);

    f32 const clamped_time = std::clamp(time, 0.f, static_cast<f32>(section_cnt));
    u32 const section = std::min(static_cast<u32>(clamped_time), section_cnt - 1);

    return {.m_section = section, .m_local_time = clamped_time - static_cast<f32>(section)};
}

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
template<typename T> sb_inline_constexpr T evaluateBezierCurve(std::span<T const> ctrl_points, f32 time);
template<typename T> sb_inline_constexpr T evaluateBezierCurve(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);

// Degree-2 polynomial Bezier curve i.e. 3 control points
template<typename T> sb_inline_constexpr T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, f32 time);
template<typename T> sb_inline_constexpr T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, std::span<f32 const, 3> weights, f32 time);

// Degree-3 polynomial Bezier curve i.e. 4 control points
template<typename T> sb_inline_constexpr T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, f32 time);
template<typename T> sb_inline_constexpr T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, std::span<f32 const, 4> weights, f32 time);

// `degree + 1` control points for the 1st section, `degree` more for each extra section
constexpr SplineLayout computeBezierSplineLayoutFromPoints(u32 degree, u32 point_cnt) {
    assert(degree > 0);
    if (point_cnt < degree + 1)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = (point_cnt - 1) / degree};
}

constexpr SplineLayout computeBezierSplineLayoutFromSections(u32 degree, u32 section_cnt) {
    assert(degree > 0);
    if (section_cnt == 0)
        return {};

    return {.m_point_cnt = degree * section_cnt + 1, .m_section_cnt = section_cnt};
}

template<typename T> sb_inline_constexpr T evaluateBezierSpline(std::span<T const> ctrl_points, u32 degree, f32 time);
template<typename T> sb_inline_constexpr T evaluateBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);
template<typename T> sb_inline_constexpr T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, f32 time);
template<typename T> sb_inline_constexpr T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);
template<typename T> sb_inline_constexpr T evaluateCubicBezierSpline(std::span<T const> ctrl_points, f32 time);

//
// Hermite
//
// Performs a cubic (degree-3) polynomial interpolation between 2 points using tangents as a guide.
// Input data can be either provided separately ([P0, .., Pn] [T0, ..., Tn]) or interleaved as keys ([{P0, T0}, ... {Pn, Tn}]).
//
// Time interval [0, 1[ cannot be linearly mapped to others because it would stretch the curve.
// To maintain the curve shape, tangents must be scaled by the target time interval e.g. [0, 5[ --> tangents * 5
//
// A Hermite spline is a combination of Hermite curves created by 1 or more extra {control point, tangent}.
//

// Interleaved Hermite data, same memory layout as [P0, T0, P1, T1, ...]
template <typename T>
struct HermiteKey {
    T m_point;
    T m_tangent;
};

template<typename T> sb_inline_constexpr T evaluateHermiteCurve(std::span<T const, 2> points, std::span<T const, 2> tangents, f32 time);
template<typename T> sb_inline_constexpr T evaluateHermiteCurve(std::span<HermiteKey<T> const, 2> keys, f32 time);

// point_cnt: number of {point, tangent}
constexpr SplineLayout computeHermiteSplineLayoutFromPoints(u32 point_cnt) {
    if (point_cnt < 2)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = point_cnt - 1};
}

constexpr SplineLayout computeHermiteSplineLayoutFromSections(u32 section_cnt) {
    if (section_cnt == 0)
        return {};

    return {.m_point_cnt = section_cnt + 1, .m_section_cnt = section_cnt};
}

template<typename T> sb_inline_constexpr T evaluateHermiteSpline(std::span<T const> points, std::span<T const> tangents, f32 time);
template<typename T> sb_inline_constexpr T evaluateHermiteSpline(std::span<HermiteKey<T> const> keys, f32 time);

//
// Cardinal
//
// Uses 4 control points (P0, P1, P2 & P3) to perform a cubic (degree-3) polynomial interpolation between the 2 middle ones (P1 & P2)
// A Cardinal spline can be extended to its endpoints by either providing them twice (P0, P0, P1, P2, P3 & P3) or using ESplineEnds::CLAMPED.
// The tension parameter indicates how straight the interpolation is ...
//  - 0: loose i.e. polynomial-like
//  - 1: straight lines
//
// Since tangents are derived from control points, scaling the time interval does not impact the curve computation/sampling.
// But this only work if the computation/sampling interval is uniform. Otherwise, the curve must be converted to Hermite and tangents scaled accordingly.
//
// A Cardinal spline is a combination of Cardinal curves created by 1 or more extra control points.
//

template<typename T> sb_inline_constexpr T evaluateCardinalCurve(std::span<T const, 4> points, f32 tension, f32 time);

constexpr SplineLayout computeCardinalSplineLayoutFromPoints(u32 point_cnt, ESplineEnds ends) {
    if (point_cnt < 4)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = ends == ESplineEnds::CLAMPED ? point_cnt - 1 : point_cnt - 3};
}

constexpr SplineLayout computeCardinalSplineLayoutFromSections(u32 section_cnt, ESplineEnds ends) {
    u32 const point_cnt = ends == ESplineEnds::CLAMPED ? section_cnt + 1 : section_cnt + 3;
    if (section_cnt == 0 || point_cnt < 4)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = section_cnt};
}

template<typename T> sb_inline_constexpr T evaluateCardinalSpline(std::span<T const> points, f32 tension, ESplineEnds ends, f32 time);

//
// Uniform BSpline
//
// Uses 4 control points (P0 .. P3) to perform a cubic approximation between P1 & P2.
// It can also be extended by adding 1 or more control points.
// A Uniform BSpline can be extended to its endpoints by either providing them three times (P0, P0, P0, P1, P2, P3, P3 & P3) or using ESplineEnds::CLAMPED.
// BSpline can be used to encode complex functions (e.g. mapping raw key frames) to its parameters.
//

constexpr SplineLayout computeBSplineLayoutFromPoints(u32 point_cnt, ESplineEnds ends) {
    if (point_cnt < 4)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = ends == ESplineEnds::CLAMPED ? point_cnt + 1 : point_cnt - 3};
}

constexpr SplineLayout computeBSplineLayoutFromSections(u32 section_cnt, ESplineEnds ends) {
    if (section_cnt == 0)
        return {};

    u32 const point_cnt = ends == ESplineEnds::CLAMPED ? section_cnt - 1 : section_cnt + 3;
    if (point_cnt < 4)
        return {};

    return {.m_point_cnt = point_cnt, .m_section_cnt = section_cnt};
}

template<typename T> sb_inline_constexpr T evaluateBSplineCurve(std::span<T const, 4> ctrl_points, f32 time);
template<typename T> sb_inline_constexpr T evaluateBSpline(std::span<T const> ctrl_points, ESplineEnds ends, f32 time);

//
// Splines conversion
//
template<typename T> sb_inline_constexpr void convertCardinalSplineToHermite(std::span<T const> points, std::span<T> hermite_points, std::span<T> hermite_tangents, f32 tension, ESplineEnds ends);
template<typename T> sb_inline_constexpr void convertCardinalSplineToHermite(std::span<T const> points, std::span<HermiteKey<T>> hermite_keys, f32 tension, ESplineEnds ends);
template<typename T> sb_inline_constexpr void convertHermiteSplineToBezier(std::span<T const> points, std::span<T const> tangents, std::span<T> bezier_points);
template<typename T> sb_inline_constexpr void convertHermiteSplineToBezier(std::span<HermiteKey<T> const> keys, std::span<T> bezier_points);

} // namespace slk

#include sb_inline_hpp("curves.inl")
