#pragma once

#include <slk/core.hpp>
#include "matrix3.hpp"
#include "matrix4.hpp"

#include <algorithm>
#include <cassert>
#include <span>

namespace slk {

template <typename T>
inline T compute_bezier_curve_at(std::span<T const> ctrl_points, slk::f32 time) {
    if (ctrl_points.size() == 1)
        return ctrl_points[0];

    const T val1 = compute_bezier_curve_at<T>(ctrl_points.subspan(0, ctrl_points.size() - 1), time);
    const T val2 = compute_bezier_curve_at<T>(ctrl_points.subspan(1, ctrl_points.size() - 1), time);

    return val1 * (1.f - time) + val2 * time;
}

template <typename T>
struct WeightedBezierRes {
    T value;
    f32 weight;
};

template <typename T>
inline WeightedBezierRes<T> compute_bezier_weighted_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time) {
    if (ctrl_points.size() == 1)
        return {ctrl_points[0], ctrl_weights[0]};

    const auto [val1, w1] =
        compute_bezier_weighted_at(ctrl_points.subspan(0, ctrl_points.size() - 1), ctrl_weights.subspan(0, ctrl_weights.size() - 1), time);
    const auto [val2, w2] =
        compute_bezier_weighted_at(ctrl_points.subspan(1, ctrl_points.size() - 1), ctrl_weights.subspan(1, ctrl_weights.size() - 1), time);
    const auto w3 = (1.f - time) * w1 + time * w2;

    return {val1 * (1.f - time) * w1 / w3 + val2 * time * w2 / w3, w3};
}

template <typename T>
inline T compute_bezier_curve_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time) {
    const auto [val, _] = compute_bezier_weighted_at(ctrl_points, ctrl_weights, time);
    return val;
}

template <typename T>
inline T compute_bezier_quadratic_curve_at(std::span<T const> ctrl_points, f32 time) {
    assert(ctrl_points.size() >= 3);

    // Polynomial form:
    //    P1*(t^2 - 2t + 1) + P2 (-2t^2 + 2t) + P3*t^2
    //
    // Matix form:
    //   (t^2, t, 1) * [  1   -2   1   0 ]  [P1]
    //                 [ -2    2   0   0 ]  [P2]
    //                 [  1    0   0   0 ]  [P3]
    //                 [  0    0   0   0 ]

    Vector3f const time_vec = {time * time, time, 1.f};
    Matrix3f const coeffs_mat = {
        1, -2, 1, -2, 2, 0, 1, 0, 0,
    };

    Vector3f const time_coeffs_vec = coeffs_mat * time_vec;

    return ctrl_points[0] * time_coeffs_vec.x + ctrl_points[1] * time_coeffs_vec.y + ctrl_points[2] * time_coeffs_vec.z;
}

template <typename T>
inline T compute_bezier_quadratic_curve_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    assert(ctrl_points.size() >= 3);

    T const weighted_ctrl_points[3] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
    };

    T const weighted_res = compute_bezier_quadratic_curve_at<T const>(weighted_ctrl_points, time);
    f32 const weights_sum = compute_bezier_quadratic_curve_at<f32 const>(weights, time);

    return weighted_res / weights_sum;
}

template <typename T>
inline T compute_bezier_cubic_curve_at(std::span<T const> ctrl_points, f32 time) {
    assert(ctrl_points.size() >= 4);

    // Polynomial form:
    //    P1*(-t^3 +3t^2 - 3t + 1) + P2*(3t^3 - 6t^2 + 3t) + P3 (-3t^3 + 3t^2) + P4*t^3
    //
    // Matix form:
    //   (t^3, t^2, t, 1) * [ -1    3   -3   1 ]  [P1]
    //                      [  3   -6    3   0 ]  [P2]
    //                      [ -3    3    0   0 ]  [P3]
    //                      [  1    0    0   0 ]  [P4]

    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    Matrix4f const coeffs_mat = 
    {
        -1, 3, -3, 1, 
        3, -6, 3, 0, 
        -3, 3, 0, 0, 
        1, 0, 0, 0
    };
    Vector4f const time_coeffs_vec =  coeffs_mat * time_vec;

    return ctrl_points[0] * time_coeffs_vec.x + ctrl_points[1] * time_coeffs_vec.y + ctrl_points[2] * time_coeffs_vec.z +
           ctrl_points[3] * time_coeffs_vec.w;
}

template <typename T>
inline T compute_bezier_cubic_curve_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    assert(ctrl_points.size() >= 4 && weights.size() >= 4);

    T const weighted_ctrl_points[4] = {
        ctrl_points[0] * weights[0],
        ctrl_points[1] * weights[1],
        ctrl_points[2] * weights[2],
        ctrl_points[3] * weights[3],
    };

    const T weighted_res = compute_bezier_cubic_curve_at<T>(weighted_ctrl_points, time);
    const f32 weighted_sum = compute_bezier_cubic_curve_at<f32 const>(weights, time);

    return weighted_res / weighted_sum;
}

template <typename T>
inline T compute_bezier_spline_at(std::span<T const> ctrl_points, u32 degree, f32 time) {
    const u32 segment_cnt = (ctrl_points.size() - 1) / degree;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bezier_curve_at<T>(ctrl_points.subspan(degree * time_fragment, degree + 1), rel_time);
}

template <typename T>
inline T compute_bezier_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time) {
    assert(ctrl_points.size() == weights.size());

    const u32 segment_cnt = (ctrl_points.size() - 1) / degree;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bezier_curve_at<T>(ctrl_points.subspan(degree * time_fragment, degree + 1), weights.subspan(degree * time_fragment, degree + 1),
                                      rel_time);
}

template <typename T>
inline T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, f32 time) {
    const u32 segment_cnt = (ctrl_points.size() - 1) / 2;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bezier_quadratic_curve_at<T>(std::span<T const>{ctrl_points.subspan(2 * time_fragment)}, rel_time);
}

template <typename T>
inline T compute_bezier_cubic_spline_at(std::span<T const> ctrl_points, f32 time) {
    const u32 segment_cnt = (ctrl_points.size() - 1) / 3;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bezier_cubic_curve_at<T>(std::span<T const>{ctrl_points.subspan(3 * time_fragment)}, rel_time);
}

template <typename T>
inline T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time) {
    const u32 segment_cnt = (ctrl_points.size() - 1) / 2;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bezier_quadratic_curve_at<T>(std::span<T const, 3>{ctrl_points.subspan(2 * time_fragment)},

                                                std::span<f32 const, 3>{weights.subspan(2 * time_fragment)}, rel_time);
}

template <typename T>
inline T compute_hermite_curve_at(std::span<T const> points, std::span<T const> tangeants, f32 time) {
    assert(points.size() >= 2 && tangeants.size() >= 2);
    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    Matrix4f const coeffs_mat = {
        {2, -2, 1, 1},
        {-3, 3, -2, -1},
        {0, 0, 1, 0},
        {1, 0, 0, 0},
    };
    Vector4f const time_coeffs_vec = coeffs_mat * time_vec ;
    return points[0] * time_coeffs_vec[0] + points[1] * time_coeffs_vec[1] + tangeants[0] * time_coeffs_vec[2] + tangeants[1] * time_coeffs_vec[3];
}

// interleaved control point and tangeants : [P0, T0] [P1, T1]
template <typename T>
inline T compute_hermite_curve_at(std::span<T const> data, f32 time) {
    assert(data.size() >= 4);

    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    Matrix4f const coeffs_mat = {
        {2, 1, -2, 1},
        {-3, -2, 3, -1},
        {0, 1, 0, 0},
        {1, 0, 0, 0},
    };
    Vector4f const time_coeffs_vec = coeffs_mat * time_vec ;
    return data[0] * time_coeffs_vec[0] + data[1] * time_coeffs_vec[1] + data[2] * time_coeffs_vec[2] + data[3] * time_coeffs_vec[3];
}

template <typename T>
inline T compute_hermite_spline_at(std::span<T const> points, std::span<T const> tangeants, f32 time) {
    const u32 segment_cnt = points.size() - 1;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_hermite_curve_at<T>(points.subspan(time_fragment), tangeants.subspan(time_fragment), rel_time);
}

template <typename T>
inline T compute_hermite_spline_at(std::span<T const> data, f32 time) {
    const u32 segment_cnt = data.size() / 2 - 1;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_hermite_curve_at<T>(data.subspan(time_fragment * 2), rel_time);
}

template <typename T>
inline T compute_cardinal_curve_at(std::span<T const> data, f32 tension, f32 time) {
    assert(data.size() >= 4);

    f32 const s = (1.f - tension) / 2.f;

    Vector4f const time_vec = {time * time * time, time * time, time, 1};
    Matrix4f const coeffs_mat = {
        {-s, 2 - s, s - 2, s},
        {2 * s, s - 3, 3 - 2 * s, -s},
        {-s, 0, s, 0},
        {0, 1, 0, 0},
    };

    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return data[0] * time_coeffs_vec[0] + data[1] * time_coeffs_vec[1] + data[2] * time_coeffs_vec[2] + data[3] * time_coeffs_vec[3];
}

template <typename T>
inline T compute_cardinal_spline_at(std::span<T const> points, f32 tension, f32 time) {
    assert(points.size() >= 4);

    const u32 segment_cnt = points.size() - 3;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_cardinal_curve_at<T>(points.subspan(time_fragment), tension, rel_time);
}

template <typename T>
inline T compute_cardinal_spline_extended_at(std::span<T const> points, f32 tension, f32 time) {
    assert(points.size() >= 4);

    const u32 segment_cnt = points.size() - 1;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    if (time_fragment == 0) {
        T const temp_data[] = {points[0], points[0], points[1], points[2]};

        return compute_cardinal_curve_at<T>(temp_data, tension, rel_time);
    } else if (time_fragment == (segment_cnt - 1)) {
        auto const point_cnt = points.size();
        T const temp_data[] = {points[point_cnt - 3], points[point_cnt - 2], points[point_cnt - 1], points[point_cnt - 1]};

        return compute_cardinal_curve_at<T>(temp_data, tension, rel_time);
    }

    return compute_cardinal_curve_at<T>(points.subspan(time_fragment - 1), tension, rel_time);
}

template <typename T>
inline void convert_hermite_spline_to_bezier(std::span<T const> points, std::span<T const> tangeants, std::span<T> bezier_data) {
    assert(points.size() >= 2 && tangeants.size() >= 2);

    slk::u32 const hermite_point_cnt = points.size();
    assert(bezier_data.size() >= (4 + 3 * (hermite_point_cnt - 2)));

    for (slk::u32 idx = 0; idx != (hermite_point_cnt - 1); idx++) {
        slk::u32 const bezier_idx = 3 * idx;
        T* const curr_bezier_data = bezier_data.data() + bezier_idx;

        curr_bezier_data[0] = points[idx];
        curr_bezier_data[1] = points[idx] + tangeants[idx] / 3.f;
        curr_bezier_data[2] = points[idx + 1] - tangeants[idx + 1] / 3.f;
        curr_bezier_data[3] = points[idx + 1];
    }
}

// interleaved control point and tangeants : [P0, T0] [P1, T1]
template <typename T>
inline void convert_hermite_spline_to_bezier(std::span<T const> data, std::span<T> bezier_data) {
    assert(data.size() >= 4 && (data.size() & 1) == 0);

    slk::u32 const hermite_point_cnt = data.size() / 2;
    assert(bezier_data.size() >= (4 + 3 * (hermite_point_cnt - 2)));

    for (slk::u32 idx = 0; idx != (hermite_point_cnt - 1); idx++) {
        slk::u32 const bezier_idx = 3 * idx;
        slk::u32 const hermite_idx = 2 * idx;
        T* const curr_bezier_data = bezier_data.data() + bezier_idx;

        curr_bezier_data[0] = data[hermite_idx];
        curr_bezier_data[1] = data[hermite_idx] + data[hermite_idx + 1] / 3.f;
        curr_bezier_data[2] = data[hermite_idx + 2] - data[hermite_idx + 3] / 3.f;
        curr_bezier_data[3] = data[hermite_idx + 2];
    }
}

template <typename T>
inline void convert_cardinal_spline_to_hermite(std::span<T const> points, std::span<T> hermite_points, std::span<T> const hermite_tangeants,
                                               f32 tension, b8 extended) {
    assert(points.size() >= 4);

    f32 const s = (1.f - tension) / 2.f;
    [[maybe_unused]] u32 const hermite_point_cnt = extended ? (points.size() * 2) >> 1 : (points.size() * 2 - 4) >> 1;
    assert(hermite_points.size() >= hermite_point_cnt && hermite_tangeants.size() >= hermite_point_cnt);

    u32 hermite_idx = extended;
    u32 cardinal_idx = 1;
    for (; cardinal_idx < (points.size() - 1); ++hermite_idx, ++cardinal_idx) {
        hermite_points[hermite_idx] = points[cardinal_idx];
        hermite_tangeants[hermite_idx] = (points[cardinal_idx + 1] - points[cardinal_idx - 1]) * s;
    }

    if (extended) {
        hermite_points[0] = points[0];
        hermite_tangeants[0] = (points[1] - points[0]) * s;

        hermite_points[hermite_idx] = points.back();
        hermite_tangeants[hermite_idx] = (points[cardinal_idx] - points[cardinal_idx - 1]) * s;
    }
}

// interleaved hermite output
template <typename T>
inline void convert_cardinal_spline_to_hermite(std::span<T const> points, std::span<T> hermite_data, f32 tension, b8 extended) {
    assert(points.size() >= 4);

    f32 const s = (1.f - tension) / 2.f;
    [[maybe_unused]] slk::u32 const hermite_item_cnt = extended ? points.size() * 2 : points.size() * 2 - 4;
    assert(hermite_data.size() >= hermite_item_cnt);

    u32 hermite_idx = extended * 2;
    u32 cardinal_idx = 1;
    for (; cardinal_idx < (points.size() - 1); hermite_idx += 2, ++cardinal_idx) {
        hermite_data[hermite_idx] = points[cardinal_idx];
        hermite_data[hermite_idx + 1] = (points[cardinal_idx + 1] - points[cardinal_idx - 1]) * s;
    }

    if (extended) {
        hermite_data[0] = points[0];
        hermite_data[1] = (points[1] - points[0]) * s;

        hermite_data[hermite_idx] = points.back();
        hermite_data[hermite_idx + 1] = (points[cardinal_idx] - points[cardinal_idx - 1]) * s;
    }
}

template <typename T>
inline T compute_bspline_cubic_section_at(std::span<T const> ctrl_points, f32 time) {
    Vector4f const time_vec = Vector4f{time * time * time, time * time, time, 1} * 1.f / 6.f;
    Matrix4f const coeffs_mat = {
        {-1, 3, -3, 1},
        {3, -6, 3, 0},
        {-3, 0, 3, 0},
        {1, 4, 1, 0},
    };

    Vector4f const time_coeffs_vec = coeffs_mat * time_vec;
    return ctrl_points[0] * time_coeffs_vec[0] + ctrl_points[1] * time_coeffs_vec[1] + ctrl_points[2] * time_coeffs_vec[2] +
           ctrl_points[3] * time_coeffs_vec[3];
}

template <typename T>
inline T compute_bspline_cubic_spline_at(std::span<T const> ctrl_points, f32 time) {
    assert(ctrl_points.size() >= 4);

    const u32 segment_cnt = ctrl_points.size() - 3;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    return compute_bspline_cubic_section_at<T>(ctrl_points.subspan(time_fragment), rel_time);
}

template <typename T>
inline T compute_bspline_cubic_spline_extended_at(std::span<T const> ctrl_points, f32 time) {
    assert(ctrl_points.size() >= 4);

    const u32 segment_cnt = ctrl_points.size() + 1;
    assert(segment_cnt > 0);

    const f32 clamped_time = std::clamp(time, 0.f, static_cast<f32>(segment_cnt));
    const u32 time_fragment = min(static_cast<u32>(clamped_time), segment_cnt - 1);
    const f32 rel_time = clamped_time - static_cast<f32>(time_fragment);

    if (time_fragment <= 1) {
        T const temp_data[] = {ctrl_points[0], ctrl_points[0], ctrl_points[time_fragment], ctrl_points[time_fragment + 1]};
        return compute_bspline_cubic_section_at<T>(temp_data, rel_time);
    } else if (time_fragment >= (segment_cnt - 2)) {
        auto const segment_offset = segment_cnt - time_fragment - 1;
        auto const point_cnt = ctrl_points.size();
        T const temp_data[] = {ctrl_points[point_cnt - segment_offset - 2], ctrl_points[point_cnt - segment_offset - 1], ctrl_points[point_cnt - 1],
                               ctrl_points[point_cnt - 1]};

        return compute_bspline_cubic_section_at<T>(temp_data, rel_time);
    }

    return compute_bspline_cubic_section_at<T>(ctrl_points.subspan(time_fragment - 2), rel_time);
}

} // namespace slk

#define SB_DEFINE_CURVE_BSPLINE(type)                                                                                                                \
    namespace slk {                                                                                                                                  \
        template type compute_bspline_cubic_section_at<type>(std::span<type const> ctrl_points, f32 time);                                           \
        template type compute_bspline_cubic_spline_at(std::span<type const> ctrl_points, f32 time);                                                  \
        template type compute_bspline_cubic_spline_extended_at(std::span<type const> ctrl_points, f32 time);                                         \
    }

#define SB_DEFINE_CURVE_BEZIER(type)                                                                                                                 \
    namespace slk {                                                                                                                                  \
        template type compute_bezier_curve_at<type>(std::span<type const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);                 \
        template type compute_bezier_curve_at<type>(std::span<type const> ctrl_points, f32 time);                                                    \
        template type compute_bezier_quadratic_spline_at<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);           \
        template type compute_bezier_cubic_spline_at<type>(std::span<type const> ctrl_points, f32 time);                                             \
        template type compute_bezier_quadratic_spline_at<type>(std::span<type const> ctrl_points, f32 time);                                         \
        template type compute_bezier_spline_at<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);         \
        template type compute_bezier_spline_at<type>(std::span<type const> ctrl_points, u32 degree, f32 time);                                       \
        template type compute_bezier_cubic_curve_at<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);                \
        template type compute_bezier_cubic_curve_at<type>(std::span<type const> ctrl_points, f32 time);                                              \
        template type compute_bezier_quadratic_curve_at<type>(std::span<type const> ctrl_points, std::span<f32 const> weights, f32 time);            \
        template type compute_bezier_quadratic_curve_at<type>(std::span<type const> ctrl_points, f32 time);                                          \
    }

#define SB_DEFINE_CURVE_HERMITE(type)                                                                                                                \
    namespace slk {                                                                                                                                  \
        template type compute_hermite_spline_at<type>(std::span<type const> data, f32 time);                                                         \
        template type compute_hermite_curve_at<type>(std::span<type const> data, f32 time);                                                          \
        template type compute_hermite_curve_at<type>(std::span<type const> points, std::span<type const> tangeants, f32 time);                       \
        template type compute_hermite_spline_at<type>(std::span<type const> points, std::span<type const> tangeants, f32 time);                      \
    }

#define SB_DEFINE_CURVE_CARDINAL(type)                                                                                                               \
    namespace slk {                                                                                                                                  \
        template type compute_cardinal_curve_at(std::span<type const> data, f32 tension, f32 time);                                                  \
        template type compute_cardinal_spline_at(std::span<type const> points, f32 tension, f32 time);                                               \
        template type compute_cardinal_spline_extended_at(std::span<type const> points, f32 tension, f32 time);                                      \
    }

#define SB_DEFINE_CURVE_CONVERSIONS(type)                                                                                                            \
    namespace slk {                                                                                                                                  \
        template void convert_cardinal_spline_to_hermite(std::span<type const> points, std::span<type> hermite_data, f32 tension, b8 extended);      \
        template void convert_cardinal_spline_to_hermite(std::span<type const> points, std::span<type> hermite_points,                               \
                                                         std::span<type> const hermite_tangeants, f32 tension, b8 extended);                         \
        template void convert_hermite_spline_to_bezier(std::span<type const> data, std::span<type> bezier_data);                                     \
        template void convert_hermite_spline_to_bezier(std::span<type const> points, std::span<type const> tangeants, std::span<type> bezier_data);  \
    }
