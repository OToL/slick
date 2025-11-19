#include "graphics.hpp"
#include "demos.hpp"

#include <cassert>
#include <chrono>

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rcamera.h>
 
int main()
{
    const int screen_width = 800*2;
    const int screen_height = 450*2;
    constexpr int grid_size = 100000;

    const auto demos = getDemos();
    [[maybe_unused]] const auto demo_count = demos.size();

    InitWindow(screen_width, screen_height, "Sandbox");

    SetTraceLogLevel(LOG_TRACE);
    DisableCursor(); // Limit cursor to relative movement inside the window
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    slk::u32 curr_demo_idx = std::to_underlying(DemoId::EMPTY);
    demos[curr_demo_idx].init();

    auto grid_hdl = graphics::create_grid();
    assert(grid_hdl);

    Camera3D camera = {{0}};
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
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};

        demos[curr_demo_idx].update(delta_time_ms, camera);

        const Matrix view_transform = GetCameraViewMatrix(&camera);
        const Matrix proj_transform = GetCameraProjectionMatrix(&camera, static_cast<float>(screen_width)/screen_height);
        const Matrix view_proj = view_transform * proj_transform;

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                graphics::render_grid(grid_hdl, cell_size, grid_size, view_proj, camera.position, thin_color, thick_color);

                if (demos[curr_demo_idx].draw3d)
                    demos[curr_demo_idx].draw3d(delta_time_ms, camera);

            EndMode3D();

            if (demos[curr_demo_idx].draw2d)
                demos[curr_demo_idx].draw2d(delta_time_ms);

            DrawRectangle(10, 10, 320, 113, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(10, 10, 320, 113, BLUE);

            char demo_tile[125];
            std::format_to(demo_tile, "Demo: {}", demos[curr_demo_idx].name);

            DrawText(demo_tile, 20, 20, 10, BLACK);
            DrawText("Free camera default controls:", 20, 40, 10, BLACK);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 40, 80, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)", 40, 100, 10, DARKGRAY);

        EndDrawing();

        const auto end_frame_time = std::chrono::high_resolution_clock::now();
        const auto frame_duration = end_frame_time - start_time_point;
        delta_time_ms = static_cast<slk::f32>(std::chrono::duration_cast<std::chrono::microseconds>(frame_duration).count() / 1000.0);
        start_time_point = end_frame_time;
    }

    demos[curr_demo_idx].deinit();

    graphics::destroy_grid(grid_hdl);
    CloseWindow();

    return 0;
}
