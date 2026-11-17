#pragma once

#include <slk/core.hpp>
#include <slk/std/span.hpp>
#include "matrix3.hpp"
#include "matrix4.hpp"
#include "utils.hpp"

#include <cassert>

import std;

namespace slk {

template <typename T>
sb_inline_constexpr T evaluateBezierCurve(std::span<T const> ctrl_points, f32 time) {
    if (ctrl_points.size() == 1)
        return ctrl_points[0];

    const T val1 = evaluateBezierCurve<T>(ctrl_points.subspan(0, ctrl_points.size() - 1), time);
    const T val2 = evaluateBezierCurve<T>(ctrl_points.subspan(1, ctrl_points.size() - 1), time);

    return val1 * (1.f - time) + val2 * time;
}

template <typename T>
struct WeightedBezierRes {
    T m_value;
    f32 m_weight;
};

template <typename T>
sb_inline_constexpr WeightedBezierRes<T> computeWeightedBezierAt(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    if (ctrl_points.size() == 1)
        return {ctrl_points[0], weights[0]};

    const auto [val1, w1] = computeWeightedBezierAt(ctrl_points.subspan(0, ctrl_points.size() - 1), weights.subspan(0, weights.size() - 1), time);
    const auto [val2, w2] = computeWeightedBezierAt(ctrl_points.subspan(1, ctrl_points.size() - 1), weights.subspan(1, weights.size() - 1), time);
    const auto w3 = (1.f - time) * w1 + time * w2;

    return {val1 * (1.f - time) * w1 / w3 + val2 * time * w2 / w3, w3};
}

template <typename T>
sb_inline_constexpr T evaluateBezierCurve(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    const auto [val, _] = computeWeightedBezierAt(ctrl_points, weights, time);
    return val;
}

template <typename T>
sb_inline_constexpr T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, f32 time) {
    // Polynomial form:
    //    P1*(t^2 - 2t + 1) + P2 (-2t^2 + 2t) + P3*t^2
    //
    // Matix form:
    //   (t^2, t, 1) * [  1   -2   1   0 ]  [P1]
    //                 [ -2    2   0   0 ]  [P2]
    //                 [  1    0   0   0 ]  [P3]
    //                 [  0    0   0   0 ]

    Vector3f const time_vec = {time * time, time, 1.f};
    constexpr Matrix3f const coeffs_mat = {
        1, -2, 1, -2, 2, 0, 1, 0, 0,
    };

    Vector3f const time_coeffs_vec = coeffs_mat * time_vec;

    return ctrl_points[0] * time_coeffs_vec.m_x + ctrl_points[1] * time_coeffs_vec.m_y + ctrl_points[2] * time_coeffs_vec.m_z;
}

template <typename T>
sb_inline_constexpr T evaluateQuadraticBezierCurve(std::span<T const, 3> ctrl_points, std::span<f32 const, 3> weights, f32 time) {
    T const weighted_ctrl_points[3] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
    };

    T const weighted_res = evaluateQuadraticBezierCurve<T const>(weighted_ctrl_points, time);
    f32 const weights_sum = evaluateQuadraticBezierCurve<f32 const>(weights, time);

    return weighted_res / weights_sum;
}

template <typename T>
sb_inline_constexpr T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, f32 time) {
    // Polynomial form:
    //    P1*(-t^3 +3t^2 - 3t + 1) + P2*(3t^3 - 6t^2 + 3t) + P3 (-3t^3 + 3t^2) + P4*t^3
    //
    // Matix form:
    //   (t^3, t^2, t, 1) * [ -1    3   -3   1 ]  [P1]
    //                      [  3   -6    3   0 ]  [P2]
    //                      [ -3    3    0   0 ]  [P3]
    //                      [  1    0    0   0 ]  [P4]

    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    constexpr Matrix4f const coeffs_mat{-1, 3, -3, 1, 3, -6, 3, 0, -3, 3, 0, 0, 1, 0, 0, 0};
    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;

    return ctrl_points[0] * time_coeffs_vec.m_x + ctrl_points[1] * time_coeffs_vec.m_y + ctrl_points[2] * time_coeffs_vec.m_z +
           ctrl_points[3] * time_coeffs_vec.m_w;
}

template <typename T>
sb_inline_constexpr T evaluateCubicBezierCurve(std::span<T const, 4> ctrl_points, std::span<f32 const, 4> weights, f32 time) {
    T const weighted_ctrl_points[4] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
        ctrl_points[3] * weights[3],
    };

    const T weighted_res = evaluateCubicBezierCurve<T>(weighted_ctrl_points, time);
    const f32 weighted_sum = evaluateCubicBezierCurve<f32 const>(weights, time);

    return weighted_res / weighted_sum;
}

template <typename T>
sb_inline_constexpr T evaluateHermiteCurve(std::span<T const, 2> points, std::span<T const, 2> tangents, f32 time) {
    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    constexpr Matrix4f const coeffs_mat{
        2, -2, 1, 1, -3, 3, -2, -1, 0, 0, 1, 0, 1, 0, 0, 0,
    };
    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return points[0] * time_coeffs_vec[0] + points[1] * time_coeffs_vec[1] + tangents[0] * time_coeffs_vec[2] + tangents[1] * time_coeffs_vec[3];
}

template <typename T>
sb_inline_constexpr T evaluateHermiteCurve(std::span<HermiteKey<T> const, 2> keys, f32 time) {
    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    constexpr Matrix4f const coeffs_mat{
        2, 1, -2, 1, -3, -2, 3, -1, 0, 1, 0, 0, 1, 0, 0, 0,
    };
    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return keys[0].m_point * time_coeffs_vec[0] + keys[0].m_tangent * time_coeffs_vec[1] + keys[1].m_point * time_coeffs_vec[2] +
           keys[1].m_tangent * time_coeffs_vec[3];
}

template <typename T>
sb_inline_constexpr T evaluateCardinalCurve(std::span<T const, 4> points, f32 tension, f32 time) {
    f32 const s = (1.f - tension) / 2.f;

    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    Matrix4f const coeffs_mat = {
        -s, 2 - s, s - 2, s, 2 * s, s - 3, 3 - 2 * s, -s, -s, 0, s, 0, 0, 1, 0, 0,
    };

    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return points[0] * time_coeffs_vec[0] + points[1] * time_coeffs_vec[1] + points[2] * time_coeffs_vec[2] + points[3] * time_coeffs_vec[3];
}

template <typename T>
sb_inline_constexpr T evaluateBezierSpline(std::span<T const> ctrl_points, u32 degree, f32 time) {
    SplineLayout const layout = computeBezierSplineLayoutFromPoints(degree, static_cast<u32>(ctrl_points.size()));
    assert(layout.m_section_cnt > 0 && "not enough control points for this degree");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    return evaluateBezierCurve<T>(ctrl_points.subspan(degree * spline_time.m_section, degree + 1), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time) {
    assert(ctrl_points.size() == weights.size());

    SplineLayout const layout = computeBezierSplineLayoutFromPoints(degree, static_cast<u32>(ctrl_points.size()));
    assert(layout.m_section_cnt > 0 && "not enough control points for this degree");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    u32 const offset = degree * spline_time.m_section;
    return evaluateBezierCurve<T>(ctrl_points.subspan(offset, degree + 1), weights.subspan(offset, degree + 1), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, f32 time) {
    SplineLayout const layout = computeBezierSplineLayoutFromPoints(2, static_cast<u32>(ctrl_points.size()));
    assert(layout.m_section_cnt > 0 && "a quadratic Bezier spline needs at least 3 control points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    return evaluateQuadraticBezierCurve<T>(makeFixSubSpan<3>(ctrl_points, 2 * spline_time.m_section), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateQuadraticBezierSpline(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    assert(ctrl_points.size() == weights.size());

    SplineLayout const layout = computeBezierSplineLayoutFromPoints(2, static_cast<u32>(ctrl_points.size()));
    assert(layout.m_section_cnt > 0 && "a quadratic Bezier spline needs at least 3 control points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    u32 const offset = 2 * spline_time.m_section;
    return evaluateQuadraticBezierCurve<T>(makeFixSubSpan<3>(ctrl_points, offset), makeFixSubSpan<3>(weights, offset), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateCubicBezierSpline(std::span<T const> ctrl_points, f32 time) {
    SplineLayout const layout = computeBezierSplineLayoutFromPoints(3, static_cast<u32>(ctrl_points.size()));
    assert(layout.m_section_cnt > 0 && "a cubic Bezier spline needs at least 4 control points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    return evaluateCubicBezierCurve<T>(makeFixSubSpan<4>(ctrl_points, 3 * spline_time.m_section), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateHermiteSpline(std::span<T const> points, std::span<T const> tangents, f32 time) {
    assert(points.size() == tangents.size());

    SplineLayout const layout = computeHermiteSplineLayoutFromPoints(static_cast<u32>(points.size()));
    assert(layout.m_section_cnt > 0 && "a Hermite spline needs at least 2 points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    return evaluateHermiteCurve<T>(makeFixSubSpan<2>(points, spline_time.m_section), makeFixSubSpan<2>(tangents, spline_time.m_section),
                                   spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateHermiteSpline(std::span<HermiteKey<T> const> keys, f32 time) {
    SplineLayout const layout = computeHermiteSplineLayoutFromPoints(static_cast<u32>(keys.size()));
    assert(layout.m_section_cnt > 0 && "a Hermite spline needs at least 2 keys");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    return evaluateHermiteCurve<T>(makeFixSubSpan<2>(keys, spline_time.m_section), spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr T evaluateCardinalSpline(std::span<T const> points, f32 tension, ESplineEnds ends, f32 time) {
    SplineLayout const layout = computeCardinalSplineLayoutFromPoints(static_cast<u32>(points.size()), ends);
    assert(layout.m_section_cnt > 0 && "a cardinal spline needs at least 4 control points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);

    if (ends == ESplineEnds::OPEN)
        return evaluateCardinalCurve<T>(makeFixSubSpan<4>(points, spline_time.m_section), tension, spline_time.m_local_time);

    // clamped: first / last sections duplicate the end points
    if (spline_time.m_section == 0) {
        T const temp_points[] = {points[0], points[0], points[1], points[2]};
        return evaluateCardinalCurve<T>(temp_points, tension, spline_time.m_local_time);
    } else if (spline_time.m_section == (layout.m_section_cnt - 1)) {
        auto const point_cnt = points.size();
        T const temp_points[] = {points[point_cnt - 3], points[point_cnt - 2], points[point_cnt - 1], points[point_cnt - 1]};
        return evaluateCardinalCurve<T>(temp_points, tension, spline_time.m_local_time);
    }

    return evaluateCardinalCurve<T>(makeFixSubSpan<4>(points, spline_time.m_section - 1), tension, spline_time.m_local_time);
}

template <typename T>
sb_inline_constexpr void convertHermiteSplineToBezier(std::span<T const> points, std::span<T const> tangents, std::span<T> bezier_points) {
    assert(points.size() >= 2 && points.size() == tangents.size());

    u32 const hermite_point_cnt = static_cast<u32>(points.size());
    assert(bezier_points.size() >= computeBezierSplineLayoutFromSections(3, hermite_point_cnt - 1).m_point_cnt);

    for (u32 idx = 0; idx != (hermite_point_cnt - 1); idx++) {
        T* const curr_bezier_points = bezier_points.data() + 3 * idx;

        curr_bezier_points[0] = points[idx];
        curr_bezier_points[1] = points[idx] + tangents[idx] / 3.f;
        curr_bezier_points[2] = points[idx + 1] - tangents[idx + 1] / 3.f;
        curr_bezier_points[3] = points[idx + 1];
    }
}

template <typename T>
sb_inline_constexpr void convertHermiteSplineToBezier(std::span<HermiteKey<T> const> keys, std::span<T> bezier_points) {
    assert(keys.size() >= 2);

    u32 const hermite_point_cnt = static_cast<u32>(keys.size());
    assert(bezier_points.size() >= computeBezierSplineLayoutFromSections(3, hermite_point_cnt - 1).m_point_cnt);

    for (u32 idx = 0; idx != (hermite_point_cnt - 1); idx++) {
        T* const curr_bezier_points = bezier_points.data() + 3 * idx;

        curr_bezier_points[0] = keys[idx].m_point;
        curr_bezier_points[1] = keys[idx].m_point + keys[idx].m_tangent / 3.f;
        curr_bezier_points[2] = keys[idx + 1].m_point - keys[idx + 1].m_tangent / 3.f;
        curr_bezier_points[3] = keys[idx + 1].m_point;
    }
}

template <typename T>
sb_inline_constexpr void convertCardinalSplineToHermite(std::span<T const> points, std::span<T> hermite_points, std::span<T> hermite_tangents, f32 tension,
                                                        ESplineEnds ends) {
    SplineLayout const cardinal_layout = computeCardinalSplineLayoutFromPoints(static_cast<u32>(points.size()), ends);
    assert(cardinal_layout.m_section_cnt > 0 && "a cardinal spline needs at least 4 control points");

    [[maybe_unused]] u32 const hermite_point_cnt = computeHermiteSplineLayoutFromSections(cardinal_layout.m_section_cnt).m_point_cnt;
    assert(hermite_points.size() >= hermite_point_cnt && hermite_tangents.size() >= hermite_point_cnt);

    f32 const s = (1.f - tension) / 2.f;
    b8 const clamped = ends == ESplineEnds::CLAMPED;

    u32 hermite_idx = clamped ? 1 : 0;
    u32 cardinal_idx = 1;
    for (; cardinal_idx < (points.size() - 1); ++hermite_idx, ++cardinal_idx) {
        hermite_points[hermite_idx] = points[cardinal_idx];
        hermite_tangents[hermite_idx] = (points[cardinal_idx + 1] - points[cardinal_idx - 1]) * s;
    }

    if (clamped) {
        hermite_points[0] = points[0];
        hermite_tangents[0] = (points[1] - points[0]) * s;

        hermite_points[hermite_idx] = points.back();
        hermite_tangents[hermite_idx] = (points[cardinal_idx] - points[cardinal_idx - 1]) * s;
    }
}

template <typename T>
sb_inline_constexpr void convertCardinalSplineToHermite(std::span<T const> points, std::span<HermiteKey<T>> hermite_keys, f32 tension, ESplineEnds ends) {
    SplineLayout const cardinal_layout = computeCardinalSplineLayoutFromPoints(static_cast<u32>(points.size()), ends);
    assert(cardinal_layout.m_section_cnt > 0 && "a cardinal spline needs at least 4 control points");

    [[maybe_unused]] u32 const hermite_point_cnt = computeHermiteSplineLayoutFromSections(cardinal_layout.m_section_cnt).m_point_cnt;
    assert(hermite_keys.size() >= hermite_point_cnt);

    f32 const s = (1.f - tension) / 2.f;
    b8 const clamped = ends == ESplineEnds::CLAMPED;

    u32 hermite_idx = clamped ? 1 : 0;
    u32 cardinal_idx = 1;
    for (; cardinal_idx < (points.size() - 1); ++hermite_idx, ++cardinal_idx) {
        hermite_keys[hermite_idx] = {.m_point = points[cardinal_idx], .m_tangent = (points[cardinal_idx + 1] - points[cardinal_idx - 1]) * s};
    }

    if (clamped) {
        hermite_keys[0] = {.m_point = points[0], .m_tangent = (points[1] - points[0]) * s};
        hermite_keys[hermite_idx] = {.m_point = points.back(), .m_tangent = (points[cardinal_idx] - points[cardinal_idx - 1]) * s};
    }
}

template <typename T>
sb_inline_constexpr T evaluateBSplineCurve(std::span<T const, 4> ctrl_points, f32 time) {
    Vector4f const time_vec = Vector4f{time * time * time, time * time, time, 1} * 1.f / 6.f;
    constexpr Matrix4f const coeffs_mat{
        -1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0,
    };

    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return ctrl_points[0] * time_coeffs_vec[0] + ctrl_points[1] * time_coeffs_vec[1] + ctrl_points[2] * time_coeffs_vec[2] +
           ctrl_points[3] * time_coeffs_vec[3];
}

template <typename T>
sb_inline_constexpr T evaluateBSpline(std::span<T const> ctrl_points, ESplineEnds ends, f32 time) {
    SplineLayout const layout = computeBSplineLayoutFromPoints(static_cast<u32>(ctrl_points.size()), ends);
    assert(layout.m_section_cnt > 0 && "a BSpline needs at least 4 control points");

    SplineTime const spline_time = splitSplineTime(time, layout.m_section_cnt);
    u32 const section = spline_time.m_section;

    if (ends == ESplineEnds::OPEN)
        return evaluateBSplineCurve<T>(makeFixSubSpan<4>(ctrl_points, section), spline_time.m_local_time);

    // clamped: first / last 2 sections repeat the end points
    if (section <= 1) {
        T const temp_points[] = {ctrl_points[0], ctrl_points[0], ctrl_points[section], ctrl_points[section + 1]};
        return evaluateBSplineCurve<T>(temp_points, spline_time.m_local_time);
    } else if (section >= (layout.m_section_cnt - 2)) {
        auto const section_offset = layout.m_section_cnt - section - 1;
        auto const point_cnt = ctrl_points.size();
        T const temp_points[] = {ctrl_points[point_cnt - section_offset - 2], ctrl_points[point_cnt - section_offset - 1], ctrl_points[point_cnt - 1],
                                 ctrl_points[point_cnt - 1]};
        return evaluateBSplineCurve<T>(temp_points, spline_time.m_local_time);
    }

    return evaluateBSplineCurve<T>(makeFixSubSpan<4>(ctrl_points, section - 2), spline_time.m_local_time);
}

} // namespace slk
