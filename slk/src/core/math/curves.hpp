#pragma once

#include <core/core.hpp>

#include <span>

namespace slk {

struct HermiteSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 tangeant_cnt = 0;
    u32 section_cnt = 0;
    u32 item_cnt = 0;
};

struct BezierSplineInfo {
    u32 ctrl_point_cnt = 0;
    u32 item_cnt = 0;
    u32 section_cnt = 0;
};

struct CardinalSplineInfo {
    slk::u32 point_cnt = 0;
    slk::u32 section_cnt = 0;
    slk::u32 item_cnt = 0;
};

CardinalSplineInfo get_cardinal_spline_info_from_points(slk::u32 point_cnt, b8 extended);
CardinalSplineInfo get_cardinal_spline_info_from_sections(slk::u32 section_cnt);

HermiteSplineInfo get_hermite_spline_info_from_points(u32 ctrl_point_cnt);
HermiteSplineInfo get_hermite_spline_info_from_sections(u32 section_cnt);

BezierSplineInfo get_bezier_spline_info_from_points(u32 section_cnt);
BezierSplineInfo get_bezier_spline_info_from_sections(u32 ctrl_point_cnt);

// interleaved data: [P0, T0] ... [Pn, Tn] Vector3f compute_bezier_at(std::span<Vector3f const>  ctrl_points, f32 time);
template<typename T> T compute_bezier_at(std::span<T const> ctrl_points, slk::f32 time);
template<typename T> T compute_bezier_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);
template<typename T> T compute_bezier_quadratic_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_quadratic_at(std::span<T const> ctrl_points, std::span<f32 const> ctrl_weights, f32 time);
template<typename T> T compute_bezier_cubic_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_cubic_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);

// // degree = polynomial degree e.g.
// //  - 2 (quadratic) : 3 control points
// //  - 3 (cubic) : 4 control points
template<typename T> T compute_bezier_spline_at(std::span<T const> ctrl_points, u32 degree, f32 time);
template<typename T> T compute_bezier_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, u32 degree, f32 time);
template<typename T> T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_cubic_spline_at(std::span<T const> ctrl_points, f32 time);
template<typename T> T compute_bezier_quadratic_spline_at(std::span<T const> ctrl_points, std::span<f32 const> weights, f32 time);

// separated data: [P0, ..., Pn] [T0, ..., Tn]
template<typename T> T compute_hermite_at(std::span<T const> points, std::span<T const>  tangeants, f32 time);
template<typename T> T compute_hermite_at(std::span<T const> data, f32 time);
template<typename T> T compute_hermite_spline_at(std::span<T const> points, std::span<T const>  tangeants, f32 time);
template<typename T> T compute_hermite_spline_at(std::span<T const> data, f32 time);

template<typename T> T compute_cardinal_at(std::span<T const> data, f32 tension, f32 time);
template<typename T> T compute_cardinal_spline_at(std::span<T const> points, f32 tension, f32 time);
template<typename T> T compute_cardinal_extended_spline_at(std::span<T const>  points, f32 tension, f32 time);

template<typename T> void convert_hermite_to_bezier(std::span<T const> points, std::span<T const>  tangeants, std::span<T> bezier_data);
template<typename T> void convert_hermite_to_bezier(std::span<T const> data, std::span<T>  bezier_data);
template<typename T> void convert_cardinal_to_hermite(std::span<T const> points, std::span<T>  hermite_points, std::span<T> const hermite_tangeants, f32 tension, b8 extended);
template<typename T> void convert_cardinal_to_hermite(std::span<T const> points, std::span<T>  hermite_data, f32 tension, b8 extended);

} // namespace slk

