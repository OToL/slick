#include "graphics.hpp"

#include <cassert>

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rcamera.h>
 
int main()
{
    const int screenWidth = 800*2;
    const int screenHeight = 450*2;
    constexpr int grid_size = 100000;

    InitWindow(screenWidth, screenHeight, "Sandbox");

    SetTraceLogLevel(LOG_TRACE);
    DisableCursor(); // Limit cursor to relative movement inside the window
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

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

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};

        const Matrix view_transform = GetCameraViewMatrix(&camera);
        const Matrix proj_transform = GetCameraProjectionMatrix(&camera, static_cast<float>(screenWidth)/screenHeight);
        const Matrix view_proj = view_transform * proj_transform;

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                graphics::render_grid(grid_hdl, cell_size, grid_size, view_proj, camera.position, thin_color, thick_color);

            EndMode3D();

            DrawRectangle(10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(10, 10, 320, 93, BLUE);

            DrawText("Free camera default controls:", 20, 20, 10, BLACK);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

        EndDrawing();
    }

    graphics::destroy_grid(grid_hdl);
    CloseWindow();

    return 0;
}
