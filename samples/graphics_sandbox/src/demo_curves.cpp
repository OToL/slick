#include "demo_curves.hpp"

#include <core/math/math.hpp>
#include <core/math/curves.hpp>
#include <core/math/vector3.hpp>

#include <raylib_utils/conversions.hpp>

#include <raylib/raylib.h>
#include <raylib/raymath.h>

#include <imgui/imgui.h>

#include <cassert>
#include <vector>
#include <algorithm>
#include <ranges>

using namespace std::string_view_literals;

// TODO
// - hermite curve+spline matrix+auto extension
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
    BEZIER,
    HERMITE,
    CARDINAL,
    _COUNT
};

enum class Test : slk::u8 {
    BEZIER_CUBIC,
    SPLINE_BEZIER_CUBIC,
    BEZIER_QUADRATIC,
    SPLINE_BEZIER_QUADRATIC,
    HERMITE,
    HERMITE_SPLINE,
    CARDINAL,
    CARDINAL_EXTENDED,
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
        {
           "Hermit",
           {
               // P0
               {1, 0.2, 1},
               // P1
               {5, 0.2, 2},

               // T0
               {0, 0, 5},
               // T1
               {-8, 0, -2},
           },
           3,
           CurveType::HERMITE
        },
        {
           "Hermit Spline",
           {
               // P0
               {1, 0.2, 1},
               // P1
               {5, 0.2, 2},
               // P2
               {3, 0.2, -2},
               // P0
               {1, 0.2, 1},

               // T0
               {0, 0, 5},
               // T1
               {0, 0, -5},
               // {-8, 0, -2},
               // T2
               {-4, 0, 0},
               // T0
               {0, 0, 5},
           },
           3,
           CurveType::HERMITE
        },
        {
           "Cardinal",
           {
               // P0
               {0, 0.2, 0},
               // P1
               {1, 0.2, 1},
               // P2
               {3, 0.2, 1},
               // P4
               {4, 0.2, 3},

           },
           3,
           CurveType::CARDINAL
        },
        {
           "Cardinal Extended",
           {
               // P0
               {0, 0.2, 0},
               {0, 0.2, 0},
               // P1
               {1, 0.2, 1},
               // P2
               {3, 0.2, 1},
               // P4
               {4, 0.2, 3},
               {4, 0.2, 3},

           },
           3,
           CurveType::CARDINAL
        },
    };
    char dbg_test_names[256] = {};

    slk::f32 curr_time = 0.f;
    slk::f32 curr_speed = 0.0001f;
    slk::i32 curr_test_idx = 0;
    slk::b8 playing_back = false;

}* g_state = nullptr;

void draw_bezier(std::span<slk::Vector3f const> const& data)
{
    slk::u32 idx = 0;
    for (slk::Vector3f const& point : data) {
        if (idx != 0) {
            DrawLine3D(slk::convert_to_rvec3(data[idx - 1]), slk::convert_to_rvec3(point), RED);
        }

        DrawCube(slk::convert_to_rvec3(point), 0.2, 0.2, 0.2, PINK);
        ++idx;
    }

}

void draw_hermite(std::span<slk::Vector3f const> const& data) {
    const slk::u32 point_cnt = data.size()/2;

    for (slk::u32 idx = 0 ; idx != point_cnt ; idx++) {
        slk::Vector3f const point = data[idx];
        slk::Vector3f const tangeant = data[idx+point_cnt];

        DrawCube(slk::convert_to_rvec3(point), 0.2, 0.2, 0.2, RED);
        DrawCylinderEx(slk::convert_to_rvec3(point), slk::convert_to_rvec3(point + tangeant), 0.03, 0.03, 100, BLUE); // Draw a cylinder with base at startPos and top at endPos
    }
}

slk::b8 init() {
    assert(!g_state);

    g_state = new State();
    g_state->curr_test_idx = std::to_underlying(Test::CARDINAL_EXTENDED);

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
    slk::f32 max_time = static_cast<slk::f32>(static_cast<slk::i32>((std::size(test_data.ctrl_points) - 1) / test_data.degree));

    if (g_state->playing_back)
        g_state->playing_back = !ImGui::Button("Stop");
    else
        g_state->playing_back = ImGui::Button("Play");

    ImGui::SliderFloat("Speed", &g_state->curr_speed, 0.f, .005f, "%.4f");

    static bool dbg_bezier_use_ref = true;
    static bool dbg_hermite_to_bezier = false;
    static bool dbg_hermite_interleave_data = false;
    static bool dbg_carndinal_to_hermite = true;
    static bool dbg_carndinal_to_extend = false;
    static slk::f32 dbg_cardinal_tension = 0.f;

    std::vector<slk::Vector3f> interleaved_data;
    std::vector<slk::Vector3f> curve_conversion;
    CurveType conversion_type = CurveType::_COUNT;

    if (test_data.type == CurveType::BEZIER)
    {
        ImGui::Checkbox("De Castelijau", &dbg_bezier_use_ref);
    }
    else if (test_data.type == CurveType::HERMITE) {
        ImGui::Checkbox("Interleave data", &dbg_hermite_interleave_data);
        ImGui::Checkbox("Convert to Bezier", &dbg_hermite_to_bezier);

        slk::u32 const point_cnt = test_data.ctrl_points.size()/2;

        if (dbg_hermite_interleave_data) {

            interleaved_data.reserve(test_data.ctrl_points.size());
            for (auto [pt, tg] : std::views::zip(std::span{test_data.ctrl_points.data(), point_cnt},
                                             std::span{test_data.ctrl_points.data() + point_cnt, point_cnt})) {
                interleaved_data.push_back(pt);
                interleaved_data.push_back(tg);
            }
        }

        if (dbg_hermite_to_bezier)
        {
            conversion_type = CurveType::BEZIER;
            slk::HermiteSplineInfo const hermite_info = slk::get_hermite_spline_info_from_points(test_data.ctrl_points.size()/2);
            slk::u32 const bezier_point_cnt = slk::get_bezier_spline_info_from_points(hermite_info.section_cnt).ctrl_point_cnt;

            curve_conversion.resize(bezier_point_cnt);

            if (dbg_hermite_interleave_data) {
                slk::convert_hermite_to_bezier<slk::Vector3f>(interleaved_data, curve_conversion);
            }
            else {
                slk::convert_hermite_to_bezier<slk::Vector3f>(std::span{test_data.ctrl_points}.subspan(0, point_cnt),
                        std::span{test_data.ctrl_points}.subspan(point_cnt),
                        curve_conversion);
            }

            max_time = point_cnt-1;
        }
        else {
            max_time = static_cast<slk::f32>((test_data.ctrl_points.size()>>1) - 1);
        }
    }
    else if (test_data.type == CurveType::CARDINAL) {

        ImGui::SliderFloat("Tension", &dbg_cardinal_tension, 0.f, 1.f, "%.3f");
        ImGui::Checkbox("Convert to Hermite", &dbg_carndinal_to_hermite);

        if (g_state->curr_test_idx == std::to_underlying(Test::CARDINAL))
            ImGui::Checkbox("Extend", &dbg_carndinal_to_extend);
        else
            dbg_carndinal_to_extend = false;

        if (dbg_carndinal_to_hermite)
            ImGui::Checkbox("Interleave data", &dbg_hermite_interleave_data);

        if (dbg_carndinal_to_hermite) {
            conversion_type = CurveType::HERMITE;

            slk::u32 const cardinal_section_cnt = test_data.ctrl_points.size() - (dbg_carndinal_to_extend?1:3);
            slk::HermiteSplineInfo hermite_curve_info = slk::get_hermite_spline_info_from_sections(cardinal_section_cnt);
            curve_conversion.resize(hermite_curve_info.item_cnt);

            slk::convert_cardinal_to_hermite<slk::Vector3f>(test_data.ctrl_points, 
                    std::span{curve_conversion}.subspan(0, hermite_curve_info.ctrl_point_cnt), 
                    std::span{curve_conversion}.subspan(hermite_curve_info.ctrl_point_cnt),
                    dbg_cardinal_tension,
                    dbg_carndinal_to_extend);

            if (dbg_hermite_interleave_data) {
                interleaved_data.reserve(curve_conversion.size());
                for (auto [pt, tg] : std::views::zip(std::span{curve_conversion}.subspan(0, hermite_curve_info.ctrl_point_cnt),
                                                     std::span{curve_conversion}.subspan(hermite_curve_info.ctrl_point_cnt))) {
                    interleaved_data.push_back(pt);
                    interleaved_data.push_back(tg);
                }
            }

            max_time = static_cast<slk::f32>(cardinal_section_cnt);
        }
        else {
            max_time = slk::get_cardinal_spline_info_from_points(test_data.ctrl_points.size(), dbg_carndinal_to_extend).section_cnt;
        }
    }

    ImGui::BeginDisabled(g_state->playing_back);
    ImGui::SliderFloat("Time", &g_state->curr_time, 0.f, max_time);
    ImGui::EndDisabled();


    ImGui::End();

    if (g_state->playing_back) {
        g_state->curr_time += g_state->curr_speed * frame_delta_ms;

        if (g_state->curr_time > max_time)
            g_state->curr_time = 0.f;
    }

    if (test_data.type == CurveType::BEZIER)
    {
        draw_bezier(test_data.ctrl_points);
    }
    else if (test_data.type == CurveType::HERMITE)
    {
        draw_hermite(test_data.ctrl_points);
    }

    if (conversion_type == CurveType::BEZIER) {
        draw_bezier(curve_conversion);
    }
    else if (conversion_type == CurveType::HERMITE) {
        draw_hermite(curve_conversion);
    }

    slk::Vector3f curr_point = {};
    if (test_data.type == CurveType::BEZIER) {

        // cubic
        if (test_data.degree == 3) {
            if (test_data.ctrl_points.size() == (test_data.degree + 1))
            {
                curr_point = dbg_bezier_use_ref ? slk::compute_bezier_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time)
                                                       : slk::compute_bezier_cubic_at<slk::Vector3f>(std::span<slk::Vector3f const, 4>{test_data.ctrl_points},
                                                                                               g_state->curr_time);

            }
            else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_point = dbg_bezier_use_ref ? slk::compute_bezier_spline_at<slk::Vector3f>(test_data.ctrl_points, 3, g_state->curr_time)
                                                       : slk::compute_bezier_cubic_spline_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time);
            }
        }
        // quadratic
        else {
            assert(test_data.degree == 2);

            if (test_data.ctrl_points.size() == (test_data.degree + 1))
            {
                curr_point = dbg_bezier_use_ref ? slk::compute_bezier_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time)
                                                       : slk::compute_bezier_quadratic_at<slk::Vector3f>(std::span<slk::Vector3f const, 3>{test_data.ctrl_points},
                                                                                                   g_state->curr_time);

            }
            else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_point = dbg_bezier_use_ref ? slk::compute_bezier_spline_at<slk::Vector3f>(test_data.ctrl_points, 2, g_state->curr_time)
                                                       : slk::compute_bezier_quadratic_spline_at<slk::Vector3f>(test_data.ctrl_points, g_state->curr_time);
            }
        }

    }
    else if (test_data.type == CurveType::HERMITE) {
        slk::u32 const point_cnt = test_data.ctrl_points.size()/2;
        // single curve
        if (test_data.ctrl_points.size() == 2) {
            if (dbg_hermite_to_bezier) {
                curr_point = slk::compute_bezier_cubic_spline_at<slk::Vector3f>(curve_conversion, g_state->curr_time);
            }
            else {
                if (dbg_hermite_interleave_data) {
                    curr_point = slk::compute_hermite_at<slk::Vector3f>(std::span<slk::Vector3f const, 4>(interleaved_data), g_state->curr_time);

                }
                else {
                    curr_point = slk::compute_hermite_at<slk::Vector3f>(std::span<slk::Vector3f const, 2>(test_data.ctrl_points),
                                                           std::span<slk::Vector3f const, 2>(test_data.ctrl_points.data() + 2, 2), g_state->curr_time);
                }
            }
        }
        // spline
        else {
            if (dbg_hermite_to_bezier) {
                curr_point = slk::compute_bezier_cubic_spline_at<slk::Vector3f>(curve_conversion, g_state->curr_time);
            }
            else {
                if (dbg_hermite_interleave_data) {
                    curr_point = slk::compute_hermite_spline_at<slk::Vector3f>(interleaved_data, g_state->curr_time);

                }
                else {
                    curr_point = slk::compute_hermite_spline_at(
                        std::span<slk::Vector3f const>(test_data.ctrl_points.data(), point_cnt),
                        std::span<slk::Vector3f const>(test_data.ctrl_points.data() + point_cnt, point_cnt), g_state->curr_time);
                }
            }
        }
    }
    else if (test_data.type == CurveType::CARDINAL) {
        for (slk::Vector3f const& point : test_data.ctrl_points) {
            DrawCube(slk::convert_to_rvec3(point), 0.2, 0.2, 0.2, BLACK);
        }

        if (dbg_carndinal_to_hermite) {
            if (curve_conversion.size() == 4)
                if (dbg_hermite_interleave_data) {
                    curr_point = slk::compute_hermite_at<slk::Vector3f>(interleaved_data, g_state->curr_time);
                }
                else {
                    curr_point = slk::compute_hermite_at<slk::Vector3f>(curve_conversion, std::span(curve_conversion.data() + 2, 2), g_state->curr_time);
                }
            else {
                assert(curve_conversion.size() > 4);
                if (dbg_hermite_interleave_data) {
                    curr_point = slk::compute_hermite_spline_at<slk::Vector3f>(interleaved_data, g_state->curr_time);
                }
                else {
                    slk::u32 const point_cnt = curve_conversion.size()/2;
                    curr_point = slk::compute_hermite_spline_at<slk::Vector3f>(std::span{curve_conversion.data(), point_cnt}, std::span(curve_conversion.data() + point_cnt, point_cnt), g_state->curr_time);
                }
            }
        }
        else {
            if ((test_data.ctrl_points.size() == 4) & !dbg_carndinal_to_extend)
            {
                curr_point = slk::compute_cardinal_at<slk::Vector3f>(test_data.ctrl_points, dbg_cardinal_tension, g_state->curr_time);
            }
            else if(dbg_carndinal_to_extend) {
                curr_point = slk::compute_cardinal_extended_spline_at<slk::Vector3f>(test_data.ctrl_points, dbg_cardinal_tension, g_state->curr_time);
            }
            else {
                curr_point = slk::compute_cardinal_spline_at<slk::Vector3f>(test_data.ctrl_points, dbg_cardinal_tension, g_state->curr_time);
            }
        }
    }

    DrawSphere(slk::convert_to_rvec3(curr_point), .15f, GREEN);

    DrawCylinderEx({0, 0, 0}, {1, 0, 0}, 0.03, 0.03, 100, GRAY);
    DrawCylinderEx({0, 0, 0}, {0, 1, 0}, 0.03, 0.03, 100, GRAY);
    DrawCylinderEx({0, 0, 0}, {0, 0, 1}, 0.03, 0.03, 100, GRAY);


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
