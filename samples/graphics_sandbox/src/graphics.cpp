#include "graphics.hpp"

#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>

struct graphics::GridHdl
{
    Mesh mesh;
    Shader shader;
    Material material;

    int grid_size_loc;
    int cell_size_loc;
    int view_proj_loc;
    int world_xform_loc;
    int camera_world_pos_loc;
    int thin_color_loc;
    int thick_color_loc;
};

graphics::GridHdl * graphics::create_grid()
{
    GridHdl * hdl = nullptr;
    Shader shader = LoadShader("samples/graphics_sandbox/data/shaders/grid_vs.glsl", "samples/graphics_sandbox/data/shaders/grid_fs.glsl");

    if (!IsShaderValid(shader))
        return nullptr;

    // Get shader uniform locations
    const int grid_size_loc = GetShaderLocation(shader, "u_gridSize");
    const int cell_size_loc = GetShaderLocation(shader, "u_cell_size");
    const int view_proj_loc = GetShaderLocation(shader, "u_viewProj");
    const int world_xform_loc = GetShaderLocation(shader, "u_worldTransform");
    const int camera_world_pos_loc = GetShaderLocation(shader, "u_cameraWorldPos");
    const int thin_color_loc = GetShaderLocation(shader, "u_thin_color");
    const int thick_color_loc = GetShaderLocation(shader, "u_thick_color");

    if (grid_size_loc < 0 || cell_size_loc < 0 || view_proj_loc < 0 || world_xform_loc < 0 || camera_world_pos_loc < 0 || thin_color_loc < 0 ||
        thick_color_loc < 0)
    {
        UnloadShader(shader);
        return nullptr;
    }

    Mesh mesh = {};

    // 4 vertices for quad
    mesh.vertexCount = 4;
    mesh.triangleCount = 2;

    // Allocate memory
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    // Vertex positions (quad from -1 to 1)
    constexpr float vertices[] = {1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f};

    // Indices for 2 triangles
    constexpr unsigned short indices[] = {
        0, 1, 2, // First triangle
        2, 3, 0 // Second triangle
    };

    memcpy(mesh.vertices, vertices, sizeof(vertices));
    memcpy(mesh.indices, indices, sizeof(indices));

    // Upload to GPU
    UploadMesh(&mesh, false);

    hdl = static_cast<GridHdl *>(malloc(sizeof(GridHdl)));

    hdl->grid_size_loc = grid_size_loc;
    hdl->cell_size_loc = cell_size_loc;
    hdl->view_proj_loc = view_proj_loc;
    hdl->world_xform_loc = world_xform_loc;
    hdl->camera_world_pos_loc = camera_world_pos_loc;
    hdl->thin_color_loc = thin_color_loc;
    hdl->thick_color_loc = thick_color_loc;

    hdl->mesh = std::move(mesh);
    hdl->shader = std::move(shader);
    hdl->material = LoadMaterialDefault();
    hdl->material.shader = hdl->shader;

    return hdl;
}

void graphics::destroy_grid(GridHdl * grid)
{
    assert(grid);

    UnloadMesh(grid->mesh);
    UnloadMaterial(grid->material);

    free(grid);
}

void graphics::render_grid(GridHdl * grid, float cell_size, float grid_size, Matrix const & view_proj, Vector3 camera_world_pos,
                           Color const & thin_color, Color const & thick_color)
{
    assert(grid);

    Matrix const grid_world_transform = MatrixIdentity();
    Vector4 const thin_colorf = {
        .x = static_cast<float>(thin_color.r) / 255,
        .y = static_cast<float>(thin_color.g) / 255,
        .z = static_cast<float>(thin_color.b) / 255,
        .w = static_cast<float>(thin_color.a) / 255,
    };
    Vector4 const thick_colorf = {
        .x = static_cast<float>(thick_color.r) / 255,
        .y = static_cast<float>(thick_color.g) / 255,
        .z = static_cast<float>(thick_color.b) / 255,
        .w = static_cast<float>(thick_color.a) / 255,
    };

    SetShaderValue(grid->shader, grid->grid_size_loc, &grid_size, SHADER_UNIFORM_FLOAT);
    SetShaderValue(grid->shader, grid->cell_size_loc, &cell_size, SHADER_UNIFORM_FLOAT);
    SetShaderValue(grid->shader, grid->camera_world_pos_loc, &camera_world_pos, SHADER_UNIFORM_VEC3);
    SetShaderValue(grid->shader, grid->thin_color_loc, &thin_colorf, SHADER_UNIFORM_VEC4);
    SetShaderValue(grid->shader, grid->thick_color_loc, &thick_colorf, SHADER_UNIFORM_VEC4);
    SetShaderValueMatrix(grid->shader, grid->view_proj_loc, view_proj);
    SetShaderValueMatrix(grid->shader, grid->world_xform_loc, grid_world_transform);

    // Disable depth writing for transparent grid
    rlDisableDepthMask();
    DrawMesh(grid->mesh, grid->material, MatrixIdentity());
    rlEnableDepthMask();
}

