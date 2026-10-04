#include "graphics.hpp"

#include <slk/math/vector3.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/color.hpp>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <bgfx/bgfx.h>

struct GridHdlImpl {
    bgfx::VertexBufferHandle vert_buff_hdl;
    bgfx::IndexBufferHandle idx_buff_hdl;
    bgfx::ProgramHandle prg_hdl;
    bgfx::UniformHandle scalar_params_u_hdl;
    bgfx::UniformHandle thin_color_u_hdl;
    bgfx::UniformHandle thick_color_u_hdl;
};

GridHdlImpl* createGrid() {
    GridHdlImpl* hdl = nullptr;

    // Source vertex and index buffers must have a longer lifetime than this method
    // because bgfx::createVertex/IndexBuffer uploads asynchronously and keeps
    // a reference to this data until the upload is done.
    static constexpr slk::Vector3f const GRID_VERTICES[] = {
        {1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, -1.0f},
        {-1.0f, 0.0f, -1.0f},
        {-1.0f, 0.0f, 1.0f},
    };
    static constexpr slk::u16 GRID_INDICES[] = {0, 1, 2, 2, 3, 0};

    auto const vs_hdl = loadShader("shaders/metal/grid_vs.sc.bin");
    auto const fs_hdl = loadShader("shaders/metal/grid_fs.sc.bin");
    auto const prg_hdl = bgfx::createProgram(vs_hdl, fs_hdl, true);
    assert(isValid(vs_hdl) && isValid(fs_hdl) && isValid(prg_hdl));

    bgfx::VertexLayout vert_decl;
    vert_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();

    auto const vert_buff_hdl = bgfx::createVertexBuffer(bgfx::makeRef(GRID_VERTICES, sizeof(GRID_VERTICES)), vert_decl);
    auto const idx_buff_hdl = bgfx::createIndexBuffer(bgfx::makeRef(GRID_INDICES, sizeof(GRID_INDICES)));
    assert(isValid(vert_buff_hdl) && isValid(idx_buff_hdl));

    auto const scalar_params_u_hdl = bgfx::createUniform("u_grid_scalar_params", bgfx::UniformType::Vec4);
    auto const thin_color_u_hdl = bgfx::createUniform("u_thin_color", bgfx::UniformType::Vec4);
    auto const thick_color_u_hdl = bgfx::createUniform("u_thick_color", bgfx::UniformType::Vec4);

    assert(isValid(thin_color_u_hdl) && isValid(thick_color_u_hdl) && isValid(scalar_params_u_hdl));

    hdl = static_cast<GridHdlImpl*>(malloc(sizeof(GridHdlImpl)));
    hdl->prg_hdl = prg_hdl;
    hdl->vert_buff_hdl = vert_buff_hdl;
    hdl->idx_buff_hdl = idx_buff_hdl;
    hdl->scalar_params_u_hdl = scalar_params_u_hdl;
    hdl->thin_color_u_hdl = thin_color_u_hdl;
    hdl->thick_color_u_hdl = thick_color_u_hdl;

    return hdl;
}

void destroyGrid(GridHdl grid) {
    assert(grid);

    bgfx::destroy(grid->prg_hdl);
    bgfx::destroy(grid->vert_buff_hdl);
    bgfx::destroy(grid->idx_buff_hdl);
    bgfx::destroy(grid->scalar_params_u_hdl);
    bgfx::destroy(grid->thin_color_u_hdl);
    bgfx::destroy(grid->thick_color_u_hdl);

    free(grid);
}

void renderGrid(GridHdl grid, slk::ColorU32 const& thin_color, slk::ColorU32 const& thick_color, bool debug_visualize) {
    assert(grid);

    const float bgfx_thin_color[4] = {thin_color.m_red / 255.f, thin_color.m_green / 255.f, thin_color.m_blue / 255.f, thin_color.m_alpha / 255.0f};
    const float bgfx_thick_color[4] = {thick_color.m_red / 255.f, thick_color.m_green / 255.f, thick_color.m_blue / 255.f, thick_color.m_alpha / 255.0f};

    // TODO: pass this by parameter
    const float grid_scalar_params[4] = {
        4000.f, // grid half size
        1.f, // cell size
        debug_visualize?1.f:0.f, // dbg viz
        0.f, // fade mode
    };

    bgfx::setTransform(slk::Matrix4f::IDENTITY.data());

    bgfx::setUniform(grid->scalar_params_u_hdl, grid_scalar_params);
    bgfx::setUniform(grid->thin_color_u_hdl, bgfx_thin_color);
    bgfx::setUniform(grid->thick_color_u_hdl, bgfx_thick_color);

    bgfx::setVertexBuffer(0, grid->vert_buff_hdl);
    bgfx::setIndexBuffer(grid->idx_buff_hdl);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA);

    bgfx::submit(0, grid->prg_hdl);
}

bgfx::ShaderHandle loadShader(const char* file_name) {
    const char* shaderPath = "???";

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Gnm:
            shaderPath = "shaders/pssl/";
            break;
        case bgfx::RendererType::Metal:
            shaderPath = "shaders/metal/";
            break;
        case bgfx::RendererType::OpenGL:
            shaderPath = "shaders/glsl/";
            break;
        case bgfx::RendererType::OpenGLES:
            shaderPath = "shaders/essl/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
    }
#pragma clang diagnostic pop

    size_t shaderLen = strlen(shaderPath);
    size_t fileLen = strlen(file_name);
    char* filePath = (char*)malloc(shaderLen + fileLen);
    memcpy(filePath, shaderPath, shaderLen);
    memcpy(&filePath[shaderLen], file_name, fileLen);

    FILE* file = fopen(file_name, "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}
