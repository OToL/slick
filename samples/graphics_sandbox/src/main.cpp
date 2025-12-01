#include "graphics.hpp"
#include "demos.hpp"

#include <cassert>
#include <chrono>
#include <utility>
#include <vector>
#include <iterator>

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rcamera.h>

#include <rlimgui/rlImGui.h>

#include <imgui/imgui.h>
 
int main()
{
    const int screen_width = 800*2;
    const int screen_height = 450*2;
    constexpr int grid_size = 1000;

    InitWindow(screen_width, screen_height, "Sandbox");

    SetTraceLogLevel(LOG_TRACE);
    DisableCursor(); // Limit cursor to relative movement inside the window
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    rlImGuiSetup(true);

    Demos::init(DemoId::CURVES);
    std::span<DemoInfo const> demos_info = Demos::getDemosInfo();
    slk::i32 curr_demo_idx = std::to_underlying(Demos::current());

    char dbg_demo_raw_names[2048] = {};
    char* dbg_buffer_iter = std::begin(dbg_demo_raw_names);
    char* const dbg_buffer_end = std::end(dbg_demo_raw_names);

    for (DemoInfo const& info : demos_info)
    {
        assert(std::distance(dbg_buffer_iter, dbg_buffer_end) > static_cast<std::ptrdiff_t>(info.name.size()));
        dbg_buffer_iter = std::copy_n(info.name.data(), info.name.size(), dbg_buffer_iter);
        *(dbg_buffer_iter++) = 0;
    }

    auto grid_hdl = graphics::create_grid();
    assert(grid_hdl);

    Camera3D camera = {};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f}; // Camera position
    camera.target = (Vector3){0.0f, 0.0f, 0.0f}; // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
    camera.fovy = 45.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

    const Color thin_color{191, 191, 191, 255};
    const Color thick_color{63, 63, 63, 255}; 
    const float cell_size = 1.f;

    slk::f32 delta_time_ms = 0.f;
    auto start_time_point = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};

        DemoInfo const& curr_demo_info = demos_info[curr_demo_idx];

        if (has_flag(curr_demo_info.caps, DemoCaps::DEFAULT_CAMERA_CONTROL) && IsKeyDown(KEY_LEFT_ALT))
            UpdateCamera(&camera, CAMERA_FREE);

        if (!Demos::update(delta_time_ms, camera))
            break;

        const Matrix view_transform = GetCameraViewMatrix(&camera);
        const Matrix proj_transform = GetCameraProjectionMatrix(&camera, static_cast<float>(screen_width)/screen_height);
        const Matrix view_proj = view_transform * proj_transform;

        BeginDrawing();

            rlImGuiBegin();

            ImGui::Begin("Demos", nullptr);

                ImGui::Spacing(); ImGui::Spacing();

                static slk::b8 show_imgui_demo = false;
                ImGui::Checkbox("Show ImGui demo", &show_imgui_demo);
                if (show_imgui_demo)
                    ImGui::ShowDemoWindow(&show_imgui_demo);

                slk::i32 next_demo_idx = curr_demo_idx;
                ImGui::TextUnformatted("Demo:"); ImGui::SameLine(); ImGui::Combo("##DemoSelection", &next_demo_idx, dbg_demo_raw_names);

                if (next_demo_idx != curr_demo_idx)
                {
                    Demos::set_current(static_cast<DemoId>(next_demo_idx));
                    curr_demo_idx = std::to_underlying(Demos::current());
                }

                ImGui::Spacing(); ImGui::Spacing();

                ImGui::TextUnformatted("Free camera default controls:");
                ImGui::TextUnformatted("- Mouse Wheel to Zoom in-out");
                ImGui::TextUnformatted("- Mouse Wheel Pressed to Pan");
                ImGui::TextUnformatted("- Z to zoom to (0, 0, 0)");

            ImGui::End();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                Demos::draw3d(delta_time_ms, camera);

                // The grid does not write to the depth buffer but is reading it
                // This means it must be rendered after opaque objects but before transparent ones
                graphics::render_grid(grid_hdl, cell_size, grid_size, view_proj, camera.position, thin_color, thick_color);

            EndMode3D();

            Demos::draw2d(delta_time_ms);

            rlImGuiEnd();

        EndDrawing();

        const auto end_frame_time = std::chrono::high_resolution_clock::now();
        const auto frame_duration = end_frame_time - start_time_point;
        delta_time_ms = static_cast<slk::f32>(std::chrono::duration_cast<std::chrono::microseconds>(frame_duration).count() / 1000.0);
        start_time_point = end_frame_time;
    }

    Demos::shutdown();

    graphics::destroy_grid(grid_hdl);
    CloseWindow();

    return 0;
}
