#pragma once

#include <core/core.hpp>
#include "math.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"
#include "vector4.hpp"
#include "matrix4.hpp"

#include <span>
#include <cassert>
#include <algorithm>

namespace slk {

// Generic Bezier curves computation using De Castelijau recursive algorithm

template <typename T>
inline T compute_bezier_at(std::span<T const> const& ctrl_points, slk::f32 time) {
    if (ctrl_points.size() == 1)
        return ctrl_points[0];

    const T val1 = compute_bezier_at(ctrl_points.subspan(0, ctrl_points.size() - 1), time);
    const T val2 = compute_bezier_at(ctrl_points.subspan(1, ctrl_points.size() - 1), time);

    return val1 * (1.f - time) + val2 * time;
}

template<typename T>
struct BezierWRes{
    T value;
    f32 weight;
};

template <typename T>
inline BezierWRes<T> compute_bezier_weighted_at(std::span<T const> ctrl_points, std::span<slk::f32 const> ctrl_weights, slk::f32 time) {
    if (ctrl_points.size() == 1)
        return {ctrl_points[0], ctrl_weights[0]};

    const auto [val1, w1] = compute_bezier_weighted_at(ctrl_points.subspan(0, ctrl_points.size() - 1), ctrl_weights.subspan(0, ctrl_weights.size() - 1), time);
    const auto [val2, w2] = compute_bezier_weighted_at(ctrl_points.subspan(1, ctrl_points.size() - 1), ctrl_weights.subspan(1, ctrl_weights.size() - 1), time);
    const auto w3 = (1.f-time)*w1 + time*w2;

    return {val1 * (1.f - time) * w1/w3 + val2 * time * w2/w3, w3};
}

template <typename T>
inline T compute_bezier_at(std::span<T const> ctrl_points, std::span<slk::f32 const> ctrl_weights, slk::f32 time) {

    const auto [val, _] = compute_bezier_weighted_at(ctrl_points, ctrl_weights, time);
    return val;
}

// Specialized (cubic & quadratic) Bezier curves computation using Bernstein polynomial

template<typename T>
inline T compute_bezier_quadratic_at(std::span<T const, 3>  ctrl_points, slk::f32 time) {

    // Polynomial form:
    //    P1*(t^2 - 2t + 1) + P2 (-2t^2 + 2t) + P3*t^2
    //
    // Matix form:
    //   (t^2, t, 1) * [  1   -2   1   0 ]  [P1]
    //                 [ -2    2   0   0 ]  [P2]
    //                 [  1    0   0   0 ]  [P3]
    //                 [  0    0   0   0 ]  

    Vector3f const time_vec = {time*time, time, 1.f};
    Matrix3f const coeffs_mat = {
         1, -2, 1,
        -2,  2, 0,
         1,  0, 0,
    };

    Vector3f const time_coeffs_vec = time_vec * coeffs_mat;

    return ctrl_points[0] * time_coeffs_vec.x +
        ctrl_points[1] * time_coeffs_vec.y +
        ctrl_points[2] * time_coeffs_vec.z;
}

template<typename T>
inline T compute_bezier_quadratic_at(std::span<T const, 3> ctrl_points, std::span<slk::f32 const, 3> weights, slk::f32 time) {

    T const weighted_ctrl_points[3] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
    };

    T const weighted_res = slk::compute_bezier_quadratic_at<T const>(weighted_ctrl_points, time);
    slk::f32 const weights_sum = slk::compute_bezier_quadratic_at(weights, time);

    return weighted_res/weights_sum;
}


template<typename T>
inline T compute_bezier_cubic_at(std::span<T const, 4> ctrl_points, slk::f32 time) {

    // Polynomial form:
    //    P1*(-t^3 +3t^2 - 3t + 1) + P2*(3t^3 - 6t^2 + 3t) + P3 (-3t^3 + 3t^2) + P4*t^3
    //
    // Matix form:
    //   (t^3, t^2, t, 1) * [ -1    3   -3   1 ]  [P1]
    //                      [  3   -6    3   0 ]  [P2]
    //                      [ -3    3    0   0 ]  [P3]
    //                      [  1    0    0   0 ]  [P4]

    Vector4f const time_vec = {time*time*time, time*time, time, 1};
    Matrix4f const coeffs_mat = {
        -1,  3, -3, 1,
         3, -6,  3, 0,
        -3,  3,  0, 0,
         1,  0,  0, 0
    };
    Vector4f const time_coeffs_vec = time_vec * coeffs_mat;

    return ctrl_points[0] * time_coeffs_vec.x +
        ctrl_points[1] * time_coeffs_vec.y +
        ctrl_points[2] * time_coeffs_vec.z +
        ctrl_points[3] * time_coeffs_vec.w;
}

template<typename T>
T compute_bezier_cubic_at(std::span<T const, 4> ctrl_points, std::span<slk::f32 const, 4> weights, slk::f32 time) {

    T const weighted_ctrl_points[4] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
        ctrl_points[3] * weights[3],
    };

    const Vector3 weighted_res = slk::compute_bezier_cubic_at(weighted_ctrl_points,time);
    const slk::f32 weighted_sum = slk::compute_bezier_cubic_at(weights,time);

    return weighted_res / weighted_sum;
}

// Splines computation

// degree = polynomial degree e.g.
//  - 2 (quadratic) : 3 control points
//  - 3 (cubic) : 4 control points
template <typename T>
inline T compute_bezier_spline_at(std::span<T const> ctrl_points, slk::u32 degree, slk::f32 time) {
    const slk::u32 segment_cnt = (ctrl_points.size() - 1) / degree;
    assert(segment_cnt > 0);

    const slk::f32 clamped_time = std::clamp(time, 0.f, static_cast<slk::f32>(segment_cnt));
    const slk::u32 time_fragment = slk::min_value(static_cast<slk::u32>(clamped_time), segment_cnt - 1);
    const slk::f32 rel_time = clamped_time - static_cast<slk::f32>(time_fragment);

    return slk::compute_bezier_at(ctrl_points.subspan(degree * time_fragment, degree + 1), rel_time);
}

template <typename T>
T compute_bezier_spline_at(std::span<T const> ctrl_points, std::span<slk::f32 const> weights, slk::u32 degree, slk::f32 time) {
    assert(ctrl_points.size() == weights.size());

    const slk::u32 segment_cnt = (ctrl_points.size() - 1) / degree;
    assert(segment_cnt > 0);

    const slk::f32 clamped_time = std::clamp(time, 0.f, static_cast<slk::f32>(segment_cnt));
    const slk::u32 time_fragment = slk::min_value(static_cast<slk::u32>(clamped_time), segment_cnt - 1);
    const slk::f32 rel_time = clamped_time - static_cast<slk::f32>(time_fragment);

    return slk::compute_bezier_at(ctrl_points.subspan(degree * time_fragment, degree + 1), weights.subspan(degree * time_fragment, degree + 1), rel_time);
}


template <typename T>
inline T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, slk::f32 time) {
    const slk::u32 segment_cnt = (ctrl_points.size() - 1) / 2;
    assert(segment_cnt > 0);

    const slk::f32 clamped_time = std::clamp(time, 0.f, static_cast<slk::f32>(segment_cnt));
    const slk::u32 time_fragment = slk::min_value(static_cast<slk::u32>(clamped_time), segment_cnt - 1);
    const slk::f32 rel_time = clamped_time - static_cast<slk::f32>(time_fragment);

    return slk::compute_bezier_quadratic_at(std::span<T const, 3>{ctrl_points.subspan(2 * time_fragment)}, rel_time);
}

template <typename T>
inline T compute_bezier_cubic_spline_at(std::span<T const> ctrl_points, slk::f32 time) {
    const slk::u32 segment_cnt = (ctrl_points.size() - 1) / 3;
    assert(segment_cnt > 0);

    const slk::f32 clamped_time = std::clamp(time, 0.f, static_cast<slk::f32>(segment_cnt));
    const slk::u32 time_fragment = slk::min_value(static_cast<slk::u32>(clamped_time), segment_cnt - 1);
    const slk::f32 rel_time = clamped_time - static_cast<slk::f32>(time_fragment);

    return slk::compute_bezier_cubic_at(std::span<T const, 4>{ctrl_points.subspan(3 * time_fragment)}, rel_time);
}

template <typename T>
inline T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, std::span<slk::f32 const> weights, slk::f32 time) {
    const slk::u32 segment_cnt = (ctrl_points.size() - 1) / 2;
    assert(segment_cnt > 0);

    const slk::f32 clamped_time = std::clamp(time, 0.f, static_cast<slk::f32>(segment_cnt));
    const slk::u32 time_fragment = slk::min_value(static_cast<slk::u32>(clamped_time), segment_cnt - 1);
    const slk::f32 rel_time = clamped_time - static_cast<slk::f32>(time_fragment);

    return slk::compute_bezier_quadratic_at(std::span<T const, 3>{ctrl_points.subspan(2 * time_fragment)}, std::span<slk::f32 const, 3>{weights.subspan(2 * time_fragment)}, rel_time);
}

} // namespace slk
