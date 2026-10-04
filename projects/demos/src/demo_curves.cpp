#include "demo_curves.hpp"

#include <slk/math/math.hpp>
#include <slk/math/curves.hpp>
#include <slk/math/utils.hpp>
#include <slk/math/vector3.hpp>
#include <slk/camera.hpp>
#include <slk/color.hpp>

#include <bgfx_utils/imgui/imgui.h>
#include <bgfx_utils/debugdraw/debugdraw.h>

#include <cassert>

import std;

using namespace slk::literals;

namespace {

enum class CurveType : slk::u8 {
    BEZIER,
    HERMITE,
    CARDINAL,
    BSPLINE,

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
    BSPLINE_SECTION,
    BSPLINE_EXTENDED,

    _COUNT
};

struct TestDesc {
    std::string_view name;
    std::vector<slk::Vector3f> ctrl_points;
    slk::u32 degree;
    CurveType type;
};

const TestDesc TEST_CURVES_DATA[std::to_underlying(Test::_COUNT)] = {
    {"Cubic Bezier",
     {
         {-5, 0, 0},
         {-3, 0, 5},
         {3, 0, 5},
         {5, 0, 0},
     },
     3,
     CurveType::BEZIER},
    {"Cubic Bezier Spline",
     {
         {-5, 0, 0},
         {-3, 0, 5},
         {3, 0, 5},
         {5, 0, 0},

         {7, 0, 5},
         {13, 0, 5},
         {15, 0, 0},
     },
     3,
     CurveType::BEZIER},
    {"Quadratic Bezier",
     {
         {-5, 0, 0},
         {-3, 0, 5},
         {3, 0, 5},
     },
     2,
     CurveType::BEZIER},
    {"Quadratic Bezier Spline",
     {
         {-5, 0, 0},
         {-3, 0, 5},
         {3, 0, 5},
         {9, 0, 5},
         {11, 0, 0},
         {9, 0, -5},
         {3, 0, -5},
         {-3, 0, -5},
         {-5, 0, 0},
     },
     2,
     CurveType::BEZIER},
    {"Hermite",
     {
         // P0
         {1, 0, 1},
         // P1
         {5, 0, 2},

         // T0
         {0, 0, 5},
         // T1
         {-8, 0, -2},
     },
     3,
     CurveType::HERMITE},
    {"Hermite Spline",
     {
         // P0
         {1, 0, 1},
         // P1
         {5, 0, 2},
         // P2
         {3, 0, -2},
         // P0
         {1, 0, 1},

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
     CurveType::HERMITE},
    {"Cardinal",
     {
         // P0
         {0, 0, 0},
         // P1
         {1, 0, 1},
         // P2
         {3, 0, 1},
         // P4
         {4, 0, 3},

     },
     3,
     CurveType::CARDINAL},
    {"Cardinal Extended",
     {
         // P0
         {0, 0, 0},
         {0, 0, 0},
         // P1
         {1, 0, 1},
         // P2
         {3, 0, 1},
         // P4
         {4, 0, 3},
         {4, 0, 3},

     },
     3,
     CurveType::CARDINAL},
    {"BSpline cubic section",
     {
         // P1
         {0, 0, 0},
         // P2
         {1, 0, 1},
         // P3
         {2, 0, 1},
         // P4
         {3, 0, 0.2},
     },
     3,
     CurveType::BSPLINE},
    {"BSpline cubic",
     {
         // P1
         {0, 0, 0},
         {0, 0, 0},
         {0, 0, 0},
         // P2
         {1, 0, 1},
         // P3
         {2, 0, 1},
         // P4
         {3, 0, 0.2},
         // P5
         {4, 0, 1.5},
         {4, 0, 1.5},
         {4, 0, 1.5},
     },
     3,
     CurveType::BSPLINE},
};

struct DemoState {
    char imgui_test_names[256] = {};

    slk::f32 curr_time = 0.f;
    slk::f32 curr_speed = 0.0001f;
    slk::f32 max_time = 0.f;
    slk::i32 curr_test_idx = 0;
    slk::b8 playing_back = false;

    slk::b8 bezier_use_ref = true;
    slk::b8 hermite_to_bezier = false;
    slk::b8 hermite_interleave_data = false;
    slk::b8 carndinal_to_hermite = true;
    slk::b8 extend_curve = false;
    slk::f32 cardinal_tension = 0.f;
};

slk::b8 init(DemoHdl& hdl) {
    DemoState* state = new DemoState{};

    state->curr_test_idx = std::to_underlying(Test::BSPLINE_SECTION);

    char* dbg_buffer_iter = std::begin(state->imgui_test_names);
    char* const dbg_buffer_end = std::end(state->imgui_test_names);

    for (TestDesc const& info : TEST_CURVES_DATA) {
        assert(std::distance(dbg_buffer_iter, dbg_buffer_end) > static_cast<std::ptrdiff_t>(info.name.size()));
        dbg_buffer_iter = std::copy_n(info.name.data(), info.name.size(), dbg_buffer_iter);
        *(dbg_buffer_iter++) = 0;
    }

    hdl = state;

    return true;
}

slk::b8 shutdown(DemoHdl hdl) {
    delete static_cast<DemoState*>(hdl);
    return true;
}

slk::b8 update(DemoHdl /* hdl */, slk::f32 /* frame_delta_ms */, slk::Camera& cam) {
    cam.m_pos.m_y = 20.f;
    cam.m_pos.m_z = .0f;
    cam.m_pos.m_x = 2.f;
    cam.m_orientation.setRotationX(90.0_deg);

    return true;
}

void draw_bezier(std::span<slk::Vector3f const> const& data, DebugDrawEncoder& dde) {
    // curve
    dde.setColor(slk::ColorU32::RED.m_abgr);
    dde.moveTo(data[0].m_x, data[0].m_y, data[0].m_z);
    for (slk::Vector3f const& point : data.subspan(1)) {
        dde.lineTo(point.m_x, point.m_y, point.m_z);
    }

    for (slk::Vector3f const& point : data) {
        dde.setColor(slk::ColorU32::PINK.m_abgr);
        slk::Vector3f const max = point + slk::Vector3f{0.1f};
        slk::Vector3f const min = point - slk::Vector3f{0.1f};

        dde.draw(bx::Aabb{.min = bx::Vec3{min.m_x, min.m_y, min.m_z}, .max = bx::Vec3{max.m_x, max.m_y, max.m_z}});
    }
}

void draw_bspline(std::span<slk::Vector3f const> const& data, DebugDrawEncoder& dde) {
    draw_bezier(data, dde);
}

void draw_hermite(std::span<slk::Vector3f const> const& data, DebugDrawEncoder& dde) {
    const slk::u32 point_cnt = data.size() / 2;

    for (slk::u32 idx = 0; idx != point_cnt; idx++) {
        slk::Vector3f const point = data[idx];
        slk::Vector3f const tangeant_end = point + data[idx + point_cnt];

        slk::Vector3f const max = point + slk::Vector3f{0.1f};
        slk::Vector3f const min = point - slk::Vector3f{0.1f};

        dde.setColor(slk::ColorU32::RED.m_abgr);
        dde.draw(bx::Aabb{.min = bx::Vec3{min.m_x, min.m_y, min.m_z}, .max = bx::Vec3{max.m_x, max.m_y, max.m_z}});

        dde.setColor(slk::ColorU32::BLUE.m_abgr);
        dde.draw(bx::Cylinder{
            .pos = bx::Vec3{point.m_x, point.m_y, point.m_z}, .end = bx::Vec3{tangeant_end.m_x, tangeant_end.m_y, tangeant_end.m_z}, .radius = .03f});
    }
}

void draw3d(DemoHdl hdl, slk::f32 frame_delta_ms, slk::Camera const& /* cam */) {
    DemoState* const app_state = static_cast<DemoState*>(hdl);

    std::vector<slk::Vector3f> interleaved_data;
    std::vector<slk::Vector3f> curve_conversion;
    [[maybe_unused]] CurveType conversion_type = CurveType::_COUNT;

    DebugDrawEncoder dde;
    dde.begin(0);

    TestDesc const& test_data = TEST_CURVES_DATA[app_state->curr_test_idx];

    if (test_data.type == CurveType::HERMITE) {
        slk::u32 const point_cnt = test_data.ctrl_points.size() / 2;

        if (app_state->hermite_interleave_data) {
            interleaved_data.reserve(test_data.ctrl_points.size());
            for (auto [pt, tg] : std::views::zip(std::span{test_data.ctrl_points.data(), point_cnt},
                                                 std::span{test_data.ctrl_points.data() + point_cnt, point_cnt})) {
                interleaved_data.push_back(pt);
                interleaved_data.push_back(tg);
            }
        }

        if (app_state->hermite_to_bezier) {
            conversion_type = CurveType::BEZIER;
            slk::HermiteSplineInfo const hermite_info = slk::computeHermiteSplineInfoFromPoints(test_data.ctrl_points.size() / 2);
            slk::u32 const bezier_point_cnt = slk::computeBezierSplineInfoFromSections(hermite_info.section_cnt).ctrl_point_cnt;

            curve_conversion.resize(bezier_point_cnt);

            if (app_state->hermite_interleave_data) {
                slk::convertHermiteSplineToBezier<slk::Vector3f>(interleaved_data, curve_conversion);
            } else {
                slk::convertHermiteSplineToBezier<slk::Vector3f>(std::span{test_data.ctrl_points}.subspan(0, point_cnt),
                                                                 std::span{test_data.ctrl_points}.subspan(point_cnt), curve_conversion);
            }
        }
    } else if (test_data.type == CurveType::CARDINAL) {
        if (app_state->carndinal_to_hermite) {
            conversion_type = CurveType::HERMITE;

            slk::u32 const cardinal_section_cnt = test_data.ctrl_points.size() - (app_state->extend_curve ? 1 : 3);
            slk::HermiteSplineInfo hermite_curve_info = slk::computeHermiteSplineInfoFromSections(cardinal_section_cnt);
            curve_conversion.resize(hermite_curve_info.item_cnt);

            slk::convertCardinalSplineToHermite<slk::Vector3f>(
                test_data.ctrl_points, std::span{curve_conversion}.subspan(0, hermite_curve_info.ctrl_point_cnt),
                std::span{curve_conversion}.subspan(hermite_curve_info.ctrl_point_cnt), app_state->cardinal_tension, app_state->extend_curve);

            if (app_state->hermite_interleave_data) {
                interleaved_data.reserve(curve_conversion.size());
                for (auto [pt, tg] : std::views::zip(std::span{curve_conversion}.subspan(0, hermite_curve_info.ctrl_point_cnt),
                                                     std::span{curve_conversion}.subspan(hermite_curve_info.ctrl_point_cnt))) {
                    interleaved_data.push_back(pt);
                    interleaved_data.push_back(tg);
                }
            }
        }
    }

    if (app_state->playing_back) {
        app_state->curr_time += app_state->curr_speed * frame_delta_ms;

        if (app_state->curr_time > app_state->max_time)
            app_state->curr_time = 0.f;
    }

    if (test_data.type == CurveType::BEZIER) {
        draw_bezier(test_data.ctrl_points, dde);
    } else if (test_data.type == CurveType::HERMITE) {
        draw_hermite(test_data.ctrl_points, dde);
    } else if (test_data.type == CurveType::BSPLINE) {
        draw_bspline(test_data.ctrl_points, dde);
    }

    if (conversion_type == CurveType::BEZIER) {
        draw_bezier(curve_conversion, dde);
    } else if (conversion_type == CurveType::HERMITE) {
        draw_hermite(curve_conversion, dde);
    }

    slk::Vector3f curr_point = {};
    if (test_data.type == CurveType::BEZIER) {
        // cubic
        if (test_data.degree == 3) {
            if (test_data.ctrl_points.size() == (test_data.degree + 1)) {
                curr_point = app_state->bezier_use_ref ? slk::evaluateBezierCurve<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time)
                                                : slk::evaluateCubicBezierCurve<slk::Vector3f>(
                                                      std::span<slk::Vector3f const, 4>{test_data.ctrl_points}, app_state->curr_time);

            } else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_point = app_state->bezier_use_ref ? slk::evaluateBezierSpline<slk::Vector3f>(test_data.ctrl_points, 3, app_state->curr_time)
                                                : slk::evaluateCubicBezierSpline<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time);
            }
        }
        // quadratic
        else {
            assert(test_data.degree == 2);

            if (test_data.ctrl_points.size() == (test_data.degree + 1)) {
                curr_point = app_state->bezier_use_ref ? slk::evaluateBezierCurve<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time)
                                                : slk::evaluateQuadraticBezierCurve<slk::Vector3f>(
                                                      std::span<slk::Vector3f const, 3>{test_data.ctrl_points}, app_state->curr_time);

            } else {
                assert(test_data.ctrl_points.size() > (test_data.degree + 1));
                curr_point = app_state->bezier_use_ref ? slk::evaluateBezierSpline<slk::Vector3f>(test_data.ctrl_points, 2, app_state->curr_time)
                                                : slk::evaluateQuadraticBezierSpline<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time);
            }
        }

    } else if (test_data.type == CurveType::HERMITE) {
        slk::u32 const point_cnt = test_data.ctrl_points.size() / 2;
        // single curve
        if (test_data.ctrl_points.size() == 2) {
            if (app_state->hermite_to_bezier) {
                curr_point = slk::evaluateCubicBezierSpline<slk::Vector3f>(curve_conversion, app_state->curr_time);
            } else {
                if (app_state->hermite_interleave_data) {
                    curr_point = slk::evaluateHermiteCurve<slk::Vector3f>(std::span<slk::Vector3f const, 4>(interleaved_data), app_state->curr_time);

                } else {
                    curr_point = slk::evaluateHermiteCurve<slk::Vector3f>(std::span<slk::Vector3f const, 2>(test_data.ctrl_points),
                                                                          std::span<slk::Vector3f const, 2>(test_data.ctrl_points.data() + 2, 2),
                                                                          app_state->curr_time);
                }
            }
        }
        // spline
        else {
            if (app_state->hermite_to_bezier) {
                curr_point = slk::evaluateCubicBezierSpline<slk::Vector3f>(curve_conversion, app_state->curr_time);
            } else {
                if (app_state->hermite_interleave_data) {
                    curr_point = slk::evaluateHermiteSpline<slk::Vector3f>(interleaved_data, app_state->curr_time);

                } else {
                    curr_point = slk::evaluateHermiteSpline(std::span<slk::Vector3f const>(test_data.ctrl_points.data(), point_cnt),
                                                            std::span<slk::Vector3f const>(test_data.ctrl_points.data() + point_cnt, point_cnt),
                                                            app_state->curr_time);
                }
            }
        }
    } else if (test_data.type == CurveType::CARDINAL) {
        for (slk::Vector3f const& point : test_data.ctrl_points) {
            dde.setColor(slk::ColorU32::BLACK.m_abgr);
            slk::Vector3f const max = point + slk::Vector3f{0.1f};
            slk::Vector3f const min = point - slk::Vector3f{0.1f};

            dde.draw(bx::Aabb{.min = bx::Vec3{min.m_x, min.m_y, min.m_z}, .max = bx::Vec3{max.m_x, max.m_y, max.m_z}});
        }

        if (app_state->carndinal_to_hermite) {
            if (curve_conversion.size() == 4)
                if (app_state->hermite_interleave_data) {
                    curr_point = slk::evaluateHermiteCurve<slk::Vector3f>(std::span<slk::Vector3f, 4>{interleaved_data}, app_state->curr_time);
                } else {
                    curr_point =
                        slk::evaluateHermiteCurve<slk::Vector3f>(std::span<slk::Vector3f, 2>{curve_conversion},
                                                                 std::span<slk::Vector3f, 2>(curve_conversion.data() + 2, 2), app_state->curr_time);
                }
            else {
                assert(curve_conversion.size() > 4);
                if (app_state->hermite_interleave_data) {
                    curr_point = slk::evaluateHermiteSpline<slk::Vector3f>(interleaved_data, app_state->curr_time);
                } else {
                    slk::u32 const point_cnt = curve_conversion.size() / 2;
                    curr_point =
                        slk::evaluateHermiteSpline<slk::Vector3f>(std::span{curve_conversion.data(), point_cnt},
                                                                  std::span(curve_conversion.data() + point_cnt, point_cnt), app_state->curr_time);
                }
            }
        } else {
            if ((test_data.ctrl_points.size() == 4) & !app_state->extend_curve) {
                curr_point =
                    slk::evaluateCardinalCurve(std::span<slk::Vector3f const, 4>{test_data.ctrl_points}, app_state->cardinal_tension, app_state->curr_time);
            } else if (app_state->extend_curve) {
                curr_point = slk::evaluateCardinalSplineExtended<slk::Vector3f>(test_data.ctrl_points, app_state->cardinal_tension, app_state->curr_time);
            } else {
                curr_point = slk::evaluateCardinalSpline<slk::Vector3f>(test_data.ctrl_points, app_state->cardinal_tension, app_state->curr_time);
            }
        }
    } else if (test_data.type == CurveType::BSPLINE) {
        if ((test_data.ctrl_points.size() == 4) & !app_state->extend_curve) {
            curr_point = slk::evaluateBSplineSection(std::span<slk::Vector3f const, 4>{test_data.ctrl_points}, app_state->curr_time);
        }
        if ((test_data.ctrl_points.size() == 4) & app_state->extend_curve) {
            curr_point = slk::evaluateBSplineExtended<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time);
        } else {
            curr_point = slk::evaluateBSpline<slk::Vector3f>(test_data.ctrl_points, app_state->curr_time);
        }
    }

    // TODO: vector3 conversion

    // Current curve interpolation
    dde.setColor(slk::ColorU32::GREEN.m_abgr);
    dde.draw(bx::Sphere{.center = bx::Vec3{curr_point.m_x, curr_point.m_y, curr_point.m_z}, .radius = .2f});

    // Axis
    dde.setColor(slk::ColorU32::RED.m_abgr);
    dde.draw(bx::Cylinder{.pos = bx::Vec3{0, 0, 0}, .end = bx::Vec3{1, 0, 0}, .radius = .1f});

    dde.setColor(slk::ColorU32::BLUE.m_abgr);
    dde.draw(bx::Cylinder{.pos = bx::Vec3{0, 0, 0}, .end = bx::Vec3{0, 0, 1}, .radius = .1f});

    dde.setColor(slk::ColorU32::GREEN.m_abgr);
    dde.draw(bx::Cylinder{.pos = bx::Vec3{0, 0, 0}, .end = bx::Vec3{0, 1, 0}, .radius = .1f});

    dde.end();
}

void draw2d(DemoHdl /* hdl */, slk::f32 /* frame_delta_ms */) { }

void drawImgui(DemoHdl hdl, slk::f32 /* frame_delta_ms */) {
    DemoState* const app_state = static_cast<DemoState*>(hdl);

    slk::i32 const prev_idx = app_state->curr_test_idx;
    ImGui::TextUnformatted("Test:");
    ImGui::SameLine();
    ImGui::Combo("##TestSelection", &app_state->curr_test_idx, app_state->imgui_test_names);

    if (prev_idx != app_state->curr_test_idx)
        app_state->curr_time = 0.f;

    TestDesc const& test_data = TEST_CURVES_DATA[app_state->curr_test_idx];
    app_state->max_time = static_cast<slk::f32>(static_cast<slk::i32>((std::size(test_data.ctrl_points) - 1) / test_data.degree));

    if (app_state->playing_back)
        app_state->playing_back = !ImGui::Button("Stop");
    else
        app_state->playing_back = ImGui::Button("Play");

    ImGui::SliderFloat("Speed", &app_state->curr_speed, 0.f, .005f, "%.4f");

    if (test_data.type == CurveType::BEZIER) {
        ImGui::Checkbox("De Castelijau", &app_state->bezier_use_ref);
    } else if (test_data.type == CurveType::HERMITE) {
        ImGui::Checkbox("Interleave data", &app_state->hermite_interleave_data);
        ImGui::Checkbox("Convert to Bezier", &app_state->hermite_to_bezier);

        slk::u32 const point_cnt = test_data.ctrl_points.size() / 2;

        if (app_state->hermite_interleave_data) {
            app_state->max_time = point_cnt - 1;
        } else {
            app_state->max_time = static_cast<slk::f32>((test_data.ctrl_points.size() >> 1) - 1);
        }
    } else if (test_data.type == CurveType::CARDINAL) {
        ImGui::SliderFloat("Tension", &app_state->cardinal_tension, 0.f, 1.f, "%.3f");
        ImGui::Checkbox("Convert to Hermite", &app_state->carndinal_to_hermite);

        if (app_state->curr_test_idx == std::to_underlying(Test::CARDINAL))
            ImGui::Checkbox("Extend", &app_state->extend_curve);
        else
            app_state->extend_curve = false;

        if (app_state->carndinal_to_hermite)
            ImGui::Checkbox("Interleave data", &app_state->hermite_interleave_data);

        if (app_state->carndinal_to_hermite) {
            slk::u32 const cardinal_section_cnt = test_data.ctrl_points.size() - (app_state->extend_curve ? 1 : 3);
            app_state->max_time = static_cast<slk::f32>(cardinal_section_cnt);
        } else {
            app_state->max_time = slk::computeCardinalSplineInfoFromPoints(test_data.ctrl_points.size(), app_state->extend_curve).section_cnt;
        }
    } else if (test_data.type == CurveType::BSPLINE) {
        if (app_state->curr_test_idx == std::to_underlying(Test::BSPLINE_SECTION))
            ImGui::Checkbox("Extend", &app_state->extend_curve);
        else
            app_state->extend_curve = false;

        app_state->max_time = slk::computeBSplineInfoFromPoints(test_data.ctrl_points.size(), app_state->extend_curve).section_cnt;
    }

    ImGui::BeginDisabled(app_state->playing_back);
    ImGui::SliderFloat("Time", &app_state->curr_time, 0.f, app_state->max_time);
    ImGui::EndDisabled();
}

} // namespace

DemoDesc curvesDemo() {
    return {.id = CURVES_DEMO_ID,
            .info = DemoInfo{.name = "Curves demo", .description = "Curves demo", .caps = DemoCapsMask::DEBUG_IMGUI},
            .api = {.init = &init, .shutdown = &shutdown, .update = &update, .drawImgui = &drawImgui, .draw3d = &draw3d, .draw2d = &draw2d}};
}
