#include "demo_picking.hpp"

#include <slk/core.hpp>
#include <raylib/raylib.h>

#include <cassert>

using namespace std::string_view_literals;

namespace {

struct State { 
    Shader picking_shader;
    int picking_loc;

} * g_state = nullptr;

using ObjectId = union {
    struct {
        slk::u32 low;
        slk::u32 high;
    };

    slk::u64 value;
};

slk::b8 init() {
    assert(!g_state);

    Shader picking_shader = LoadShader(nullptr, "samples/graphics_sandbox/data/shaders/base_picking_fs.glsl");

    if (!IsShaderValid(picking_shader))
        return false;

    const int picking_coord_loc = GetShaderLocation(picking_shader, "u_picking_coord");
    // const int object_id_loc = GetShaderLocation(picking_shader, "u_obj_identity");

    if (picking_coord_loc < 0)
        return false;

    g_state = new State{};
    g_state->picking_shader = picking_shader;
    g_state->picking_loc = picking_coord_loc;

    return true;
}

slk::b8 shutdown() {
    assert(g_state);

    UnloadShader(g_state->picking_shader);

    delete g_state;
    g_state = nullptr;

    return true;
}

slk::b8 update(slk::f32, Camera3D&) {
    return true;
}

slk::b8 draw3d(slk::f32, Camera3D const&) {


    const slk::i32 cursor_pos [] = {-1, -1};
    SetShaderValue(g_state->picking_shader, g_state->picking_loc, cursor_pos, SHADER_UNIFORM_IVEC2);

    BeginShaderMode(g_state->picking_shader);

        DrawCube({0, 0, 0}, 0.5, 0.5, 0.5, RED);
        DrawCube({2, 0, 0}, 0.5, 0.5, 0.5, GREEN);
        DrawCube({-2, 0, 0}, 0.5, 0.5, 0.5, BLUE);

    EndShaderMode();

    return true;
}
} // namespace

DemoInfo DemoPicking::getInfo() {
    return {
        .name = "Picking"sv,
        .description = "Pixel Perfect Picking Test"sv,
        .caps = FDemoCaps::DEFAULT_CAMERA_CONTROL,
    };
}

DemoFn DemoPicking::getFn() {
    return {
        .init = &init,
        .shutdown = &shutdown,
        .update = &update,
        .draw3d = &draw3d,
        .draw2d = nullptr,
    };
}
