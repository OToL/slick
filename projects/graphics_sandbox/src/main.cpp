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
#include <raylib/rlgl.h>

#include <rlimgui/rlImGui.h>

#include <imgui/imgui.h>

#define CAMERA_MOVE_SPEED                               50.4f       // Units per second
#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_ORBITAL_SPEED                            0.5f       // Radians per second
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f


void UpdateCamera2(Camera *camera, int mode)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FREE) || (mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = false;

    // Camera speeds based on frame time
    float cameraMoveSpeed = CAMERA_MOVE_SPEED*GetFrameTime();
    float cameraRotationSpeed = CAMERA_ROTATION_SPEED*GetFrameTime();
    float cameraPanSpeed = CAMERA_PAN_SPEED*GetFrameTime();
    float cameraOrbitalSpeed = CAMERA_ORBITAL_SPEED*GetFrameTime();

    if (mode == CAMERA_CUSTOM) {}
    else if (mode == CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(GetCameraUp(camera), cameraOrbitalSpeed);
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    }
    else
    {
        // Camera rotation
        if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) CameraPitch(camera, cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -cameraRotationSpeed, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, cameraRotationSpeed, rotateAroundTarget);
        if (IsKeyDown(KEY_Q)) CameraRoll(camera, -cameraRotationSpeed);
        if (IsKeyDown(KEY_E)) CameraRoll(camera, cameraRotationSpeed);

        // Camera movement
        // Camera pan (for CAMERA_FREE)
        if ((mode == CAMERA_FREE) && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)))
        {
            const Vector2 mouseDelta = GetMouseDelta();
            if (mouseDelta.x > 0.0f) CameraMoveRight(camera, cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.x < 0.0f) CameraMoveRight(camera, -cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.y > 0.0f) CameraMoveUp(camera, -cameraPanSpeed);
            if (mouseDelta.y < 0.0f) CameraMoveUp(camera, cameraPanSpeed);
        }
        else
        {
            // Mouse support
            CameraYaw(camera, -mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);
        }

        // Keyboard support
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_D)) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);

        // Gamepad movement
        if (IsGamepadAvailable(0))
        {
            // Gamepad controller support
            CameraYaw(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)*2)*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)*2)*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);
        }

        if (mode == CAMERA_FREE)
        {
            if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, cameraMoveSpeed);
            if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -cameraMoveSpeed);
        }
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL) || (mode == CAMERA_FREE))
    {
        // Zoom target distance
        CameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT)) CameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD)) CameraMoveToTarget(camera, -2.0f);
    }
}


// TODO
// - Picking
// - Handles e.g. Translation (axis and plane), rotation, etc.
 
int main()
{
    const int screen_width = 800*2;
    const int screen_height = 450*2;
    constexpr int grid_half_size = 4000;

    InitWindow(screen_width, screen_height, "Sandbox");

    SetTraceLogLevel(LOG_TRACE);
    DisableCursor(); // Limit cursor to relative movement inside the window
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    rlImGuiSetup(true);

    Demos::init(EDemoId::PICKING);
    std::span<DemoInfo const> demos_info = Demos::getDemosInfo();
    slk::i32 curr_demo_idx = std::to_underlying(Demos::current());

    char dbg_demo_raw_names[2048] = {};
    char* dbg_buffer_iter = std::begin(dbg_demo_raw_names);
    [[maybe_unused]] char* const dbg_buffer_end = std::end(dbg_demo_raw_names);

    for (DemoInfo const& info : demos_info)
    {
        assert(std::distance(dbg_buffer_iter, dbg_buffer_end) > static_cast<std::ptrdiff_t>(info.name.size()));
        dbg_buffer_iter = std::copy_n(info.name.data(), info.name.size(), dbg_buffer_iter);
        *(dbg_buffer_iter++) = 0;
    }

    bool dbg_grid_viz = true;
    auto grid_hdl = graphics::create_grid();
    assert(grid_hdl);

    Camera3D camera = {};
    camera.position = (Vector3){0.0f, 60.0f, 30.0f}; // Camera position

    camera.target = (Vector3){0.0f, 0.0f, 0.0f}; // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
    camera.fovy = 45.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

    const Color thin_color{161, 161, 161, 255};
    const Color thick_color{83, 83, 83, 255}; 
    const float cell_size = 1.f;

    slk::f32 delta_time_ms = 0.f;
    auto start_time_point = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};

        DemoInfo const& curr_demo_info = demos_info[curr_demo_idx];

        if ((curr_demo_info.caps &  FDemoCaps::DEFAULT_CAMERA_CONTROL) && IsKeyDown(KEY_LEFT_ALT))
            UpdateCamera2(&camera, CAMERA_FREE);

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
                    Demos::set_current(static_cast<EDemoId>(next_demo_idx));
                    curr_demo_idx = std::to_underlying(Demos::current());
                }
                ImGui::Checkbox("Debug grid visualize", &dbg_grid_viz);

                ImGui::Spacing(); ImGui::Spacing();

                ImGui::TextUnformatted("Free camera default controls:");
                ImGui::TextUnformatted("- Mouse Wheel to Zoom in-out");
                ImGui::TextUnformatted("- Mouse Wheel Pressed to Pan");
                ImGui::TextUnformatted("- Z to zoom to (0, 0, 0)");

            ImGui::End();

            ClearBackground(LIGHTGRAY);

            BeginMode3D(camera);
                Demos::draw3d(delta_time_ms, camera);
            EndMode3D();

            BeginMode3D(camera);
                graphics::render_grid(grid_hdl, cell_size, grid_half_size, view_proj, camera.position, thin_color, thick_color, dbg_grid_viz);
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
