#include "demo_curves.hpp"

#include <core/math/math.hpp>
#include <core/math/bezier.hpp>

#include <raylib_utils/conversion.hpp>

#include <raylib/raylib.h>
#include <raylib/raymath.h>

#include <imgui/imgui.h>

#include <cassert>
#include <vector>
#include <algorithm>

using namespace std::string_view_literals;

// TODO
// - Use slk math instead of Raylib
// - More curves
// - Picking
// - Translation handle

// Transforms a Vector3 by a given Matrix
Vector4 Vector4Transform(Vector4 v, Matrix mat)
{
    Vector4 result = {};

    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12*w;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13*w;
    result.z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14*w;
    result.w = mat.m3*x + mat.m7*y + mat.m11*z + mat.m15*w;

    return result;
}

namespace {

enum class CurveType : slk::u8 {
    BEZIER
};

enum class Test : slk::u8 {
    BEZIER_CUBIC,
    SPLINE_BEZIER_CUBIC,
    BEZIER_QUADRATIC,
    SPLINE_BEZIER_QUADRATIC,
    _COUNT
};

struct TestDesc {
    std::string_view name;
    std::vector<slk::Vector3f> ctrl_points;
    slk::u32 degree;
    CurveType type;
};

struct State {
    const TestDesc test_data[std::to_underlying(Test::_COUNT)] = {
        {
            "Cubic Bezier",
           {
               {-5, 0.2, 0},
               {-3, 0.2, 5},
               {3, 0.2, 5},
               {5, 0.2, 0},
           },
           3,
           CurveType::BEZIER
        },
        {
            "Cubic Bezier Spline",
            {
               {-5, 0.2, 0},
               {-3, 0.2, 5},
               {3, 0.2, 5},
               {5, 0.2, 0},

               {7, 0.2, 5},
               {13, 0.2, 5},
               {15, 0.2, 0},
            },
            3,
            CurveType::BEZIER
        },
        {
           "Quadratic Bezier",
           {
               {-5, 0.2, 0},
               {-3, 0.2, 5},
               {3, 0.2, 5},
           },
           2,
           CurveType::BEZIER
        },
        {
           "Quadratic Bezier Spline",
           {
               {-5, 0.2, 0},
               {-3, 0.2, 5},
               {3, 0.2, 5},
               {9, 0.2, 5},
               {11, 0.2, 0},
               {9, 0.2, -5},
               {3, 0.2, -5},
               {-3, 0.2, -5},
               {-5, 0.2, 0},
           },
           2,
           CurveType::BEZIER
        },

    };
    char dbg_test_names[256] = {};

    slk::f32 curr_time = 0.f;
    slk::f32 curr_speed = 0.0001f;
    slk::i32 curr_test_idx = 0;
    slk::b8 playing_back = false;

}* g_state = nullptr;

slk::b8 init() {
    assert(!g_state);

    g_state = new State();

    char* dbg_buffer_iter = std::begin(g_state->dbg_test_names);
    char* const dbg_buffer_end = std::end(g_state->dbg_test_names);
    for (TestDesc const& info : g_state->test_data)
    {
        assert(std::distance(dbg_buffer_iter, dbg_buffer_end) > static_cast<std::ptrdiff_t>(info.name.size()));
        dbg_buffer_iter = std::copy_n(info.name.data(), info.name.size(), dbg_buffer_iter);
        *(dbg_buffer_iter++) = 0;
    }

    return true;
}

slk::b8 shutdown() {
    assert(g_state);

    return true;
}

slk::b8 update(slk::f32 /* frame_delta_ms */, Camera3D& /* cam3d */) {

    return true;
}

slk::b8 draw3d(slk::f32 frame_delta_ms, Camera3D const& /* cam3d */) {

    ImGui::Begin("Curves", nullptr);

    slk::i32 const prev_idx = g_state->curr_test_idx;
    ImGui::TextUnformatted("Test:"); ImGui::SameLine(); ImGui::Combo("##TestSelection", &g_state->curr_test_idx, g_state->dbg_test_names);

    if (prev_idx != g_state->curr_test_idx)
        g_state->curr_time = 0.f;

    TestDesc const& test_data = g_state->test_data[g_state->curr_test_idx];
    const slk::f32 max_time = static_cast<slk::f32>(static_cast<slk::i32>((std::size(test_data.ctrl_points) - 1) / test_data.degree));

    if (g_state->playing_back)
        g_state->playing_back = !ImGui::Button("Stop");
    else
        g_state->playing_back = ImGui::Button("Play");

    ImGui::BeginDisabled(g_state->playing_back);
    ImGui::SliderFloat("Time", &g_state->curr_time, 0.f, max_time);
    ImGui::EndDisabled();

    ImGui::SliderFloat("Speed", &g_state->curr_speed, 0.f, .005f, "%.4f");

    static bool dbg_bezier_use_ref = true;
    if (test_data.type == CurveType::BEZIER)
    {
        ImGui::Checkbox("De Castelijau", &dbg_bezier_use_ref);
    }

    ImGui::End();

    if (g_state->playing_back) {
        g_state->curr_time += g_state->curr_speed * frame_delta_ms;

        if (g_state->curr_time > max_time)
            g_state->curr_time = 0.f;
    }

    slk::u32 idx = 0;
    for (slk::Vector3f const& point : test_data.ctrl_points) {
        if (idx != 0) {
            DrawLine3D(slk::to_rvec3(test_data.ctrl_points[idx - 1]), slk::to_rvec3(point), RED);
        }

        DrawCube(slk::to_rvec3(point), 0.5, 0.5, 0.5, RED);
        ++idx;
    }

    if (test_data.type == CurveType::BEZIER) {

        slk::Vector3f curr_bezier_point = {};

        // cubic
        if (test_data.degree == 3) {
            if (test_data.ctrl_points.size() == (test_data.degree + 1))
            {
                curr_bezier_point = dbg_bezier_use_ref ? slk::compute_bezier_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time)
                                                       : slk::compute_bezier_cubic_at<slk::Vector3f>(std::span<slk::Vector3f const, 4>{test_data.ctrl_points},
                                                                                               g_state->curr_time);

            }
            else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_bezier_point = dbg_bezier_use_ref ? slk::compute_bezier_spline_at<slk::Vector3f>(test_data.ctrl_points, 3, g_state->curr_time)
                                                       : slk::compute_bezier_cubic_spline_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time);
            }
        }
        // quadratic
        else {
            assert(test_data.degree == 2);

            if (test_data.ctrl_points.size() == (test_data.degree + 1))
            {
                curr_bezier_point = dbg_bezier_use_ref ? slk::compute_bezier_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time)
                                                       : slk::compute_bezier_quadratic_at<slk::Vector3f>(std::span<slk::Vector3f const, 3>{test_data.ctrl_points},
                                                                                                   g_state->curr_time);

            }
            else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_bezier_point = dbg_bezier_use_ref ? slk::compute_bezier_spline_at<slk::Vector3f>(test_data.ctrl_points, 2, g_state->curr_time)
                                                       : slk::compute_bezier_quadratic_spline_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time);
            }
        }

        DrawSphere(slk::to_rvec3(curr_bezier_point), .2f, GREEN);
    }

    // slk::f32 const weights[] = {1.f, 2.f, 1.f};
    // const auto [curr_bezier_ref, _] = compute_bezier_at(g_state->control_points, weights, g_state->curr_time);
    // const auto curr_bezier = compute_bezier_quadratic_at<Vector3>(g_state->control_points, weights, g_state->curr_time);

    // slk::f32 const weights[] = {1.f, 2.f, 2.f, 1.f};
    // const auto [curr_bezier_ref, _] = compute_bezier_at(g_state->control_points, weights, g_state->curr_time);
    // const auto curr_bezier = compute_bezier_cubic_at(g_state->control_points, weights, g_state->curr_time);

    // const Vector3 curr_bezier = compute_bezier_cubic_at(g_state->control_points, g_state->curr_time);
    // const Vector3 curr_bezier_ref = compute_bezier_at(g_state->control_points, g_state->curr_time);

    // const Vector3 curr_bezier = compute_bezier_quadratic_spline_at(g_state->control_points, g_state->curr_time);
    // const Vector3 curr_bezier_ref = compute_bezier_spline_at(g_state->control_points, 2, g_state->curr_time);

    // slk::f32 const weights[] = {1.f, 5.f, 1.f, 5.f, 1.f};
    // const Vector3 curr_bezier = compute_bezier_quadratic_spline_at(g_state->control_points, weights, g_state->curr_time);
    // const auto [curr_bezier_ref, _] = compute_bezier_spline_at(g_state->control_points, weights, 2, g_state->curr_time);

    // const Vector3 curr_bezier_ref = compute_bezier_spline_at(g_state->control_points, 3, g_state->curr_time);
    // const Vector3 curr_bezier = compute_bezier_cubic_spline_at(g_state->control_points, g_state->curr_time);

    // const Vector3 curr_bezier_ref = compute_bezier_at(g_state->control_points, g_state->curr_time);
    // const Vector3 curr_bezier = compute_bezier_quadratic_at<Vector3>(g_state->control_points, g_state->curr_time);

    // const Vector3 curr_bezier = compute_bezier_at(g_state->control_points, g_state->curr_time);
    // DrawSphere(curr_bezier, .2f, GREEN);
    // DrawSphere(curr_bezier_ref, .2f, BLUE);

    return true;
}
} // namespace

DemoInfo DemoCurves::getInfo() {
    return {
        .name = "Curves"sv,
        .description = "Curves (Bezier, etc.) test"sv,
        .caps = DemoCaps::DEFAULT_CAMERA_CONTROL,
    };
}

DemoFn DemoCurves::getFn() {
    return {
        .init = &init,
        .shutdown = &shutdown,
        .update = &update,
        .draw3d = &draw3d,
        .draw2d = nullptr,
    };
}
