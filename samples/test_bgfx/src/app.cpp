#include "app.hpp"

#include <slk/math/vector3.hpp>
#include <slk/math/matrix2.hpp>
#include <slk/math/matrix3.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/input/input.hpp>
#include <slk/input/events.hpp>
#include <slk/camera.hpp>
#include <slk/color.hpp>

#include <sokol/sokol_app.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <bgfx_utils/imgui/imgui.h>

#include <cstring>
#include <cstdio>
#include <cassert>

using namespace slk::literals;

struct PosColVertex {
    slk::Vector3f pos;
    slk::u32 color;
};

constexpr slk::Vector3f const GRID_VERTICES[] = {
    {1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, -1.0f},
    {-1.0f, 0.0f, -1.0f},
    {-1.0f, 0.0f, 1.0f},
};

constexpr slk::u16 GRID_INDICES[] = {0, 1, 2, 2, 3, 0};

// TODO: add missing colors to ColorU32
constexpr static const PosColVertex CUBE_VERTICES[] = {
    {{-1.f, 1.f, 1.f}, slk::ColorU32::black().abgr},
    {{1.f, 1.f, 1.f}, slk::ColorU32::red().abgr},
    {{-1.f, -1.f, 1.f}, slk::ColorU32::green().abgr},
    {{1.f, -1.f, 1.f}, 0xFF00FFFFU},
    {{-1.f, 1.f, -1.f}, slk::ColorU32::blue().abgr},
    {{1.f, 1.f, -1.f}, 0xFFFF00FFU},
    {{-1.f, -1.f, -1.f}, 0xFFFFFF00U},
    {{1.f, -1.f, -1.f}, slk::ColorU32::white().abgr},
};

constexpr static const slk::u16 CUBE_INDICES[] = {
    0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6, 1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};

// TODO: move and rewrite e.g. string view, etc.
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

struct Grid {
    bgfx::VertexBufferHandle vert_buff_hdl;
    bgfx::IndexBufferHandle idx_buff_hdl;
    bgfx::VertexLayout vert_decl;
    bgfx::ProgramHandle prg_hdl;

    bgfx::UniformHandle grid_scalar_params_u_hdl;
    bgfx::UniformHandle thin_color_u_hdl;
    bgfx::UniformHandle thick_color_u_hdl;
};

struct State {
    bgfx::VertexBufferHandle vert_buff_hdl;
    bgfx::IndexBufferHandle idx_buff_hdl;
    bgfx::ProgramHandle prg_hdl;
    bgfx::VertexLayout vert_decl;

    Grid grid;

    slk::Matrix3f camera_rotation;
    slk::Vector4f camera_position;
} g_state;

void app_init(void) {
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count;
    bgfx_init.resolution.width = sapp_width();
    bgfx_init.resolution.height = sapp_height();
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData.nwh = app_get_window_hdl();

    bgfx::init(bgfx_init);

    bgfx::ShaderHandle vs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/vs_cubes.bin");
    bgfx::ShaderHandle fs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/fs_cubes.bin");
    g_state.prg_hdl = bgfx::createProgram(vs_hdl, fs_hdl, true);

    assert(isValid(vs_hdl) && isValid(fs_hdl) && isValid(g_state.prg_hdl));

    g_state.vert_decl.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    g_state.vert_buff_hdl = bgfx::createVertexBuffer(bgfx::makeRef(CUBE_VERTICES, sizeof(CUBE_VERTICES)), g_state.vert_decl);
    g_state.idx_buff_hdl = bgfx::createIndexBuffer(bgfx::makeRef(CUBE_INDICES, sizeof(CUBE_INDICES)));
    assert(isValid(g_state.vert_buff_hdl) && isValid(g_state.idx_buff_hdl));

    Grid grid;
    grid.vert_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();

    grid.vert_buff_hdl = bgfx::createVertexBuffer(bgfx::makeRef(GRID_VERTICES, sizeof(GRID_VERTICES)), grid.vert_decl);
    grid.idx_buff_hdl = bgfx::createIndexBuffer(bgfx::makeRef(GRID_INDICES, sizeof(GRID_INDICES)));
    assert(isValid(grid.vert_buff_hdl) && isValid(grid.idx_buff_hdl));

    vs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/grid_vs.sc.bin");
    fs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/grid_fs.sc.bin");
    grid.prg_hdl = bgfx::createProgram(vs_hdl, fs_hdl, true);
    assert(isValid(vs_hdl) && isValid(fs_hdl) && isValid(g_state.prg_hdl));

    grid.grid_scalar_params_u_hdl = bgfx::createUniform("u_grid_scalar_params", bgfx::UniformType::Vec4);
    grid.thin_color_u_hdl = bgfx::createUniform("u_thin_color", bgfx::UniformType::Vec4);
    grid.thick_color_u_hdl = bgfx::createUniform("u_thick_color", bgfx::UniformType::Vec4);

    assert(isValid(grid.thin_color_u_hdl) && isValid(grid.thick_color_u_hdl) && isValid(grid.grid_scalar_params_u_hdl));

    g_state.grid = grid;

    slk::Matrix4f view;
    const slk::Vector3f at = {0.0f, 0.0f, 0.0f};
    const slk::Vector3f eye = {10.0f, 10.0f, 10.0f};
    view.setLookAt(eye, at, slk::Vector3f::UNITY);
    view.inverse(); // view matrix transforms world into camera space i.e. camera transform is the inverse

    g_state.camera_position = view.getTranslation();
    g_state.camera_rotation = view.getRotation();

    imguiCreate();

    slk::InputAPI::init();
}

void renderSlickWindow() {
    slk::Vector2f const mouse_pos = slk::InputAPI::getMousePosition();
    slk::Vector2f const mouse_scroll = slk::InputAPI::getMouseScroll();
    slk::MouseButton::Type const mouse_buttons_state = slk::InputAPI::getMouseButtonsState();
    slk::u8 const imgui_button_state = (mouse_buttons_state & slk::MouseButton::mask(slk::MouseButton::LEFT) ? IMGUI_MBUT_LEFT : 0) |
                                       (mouse_buttons_state & slk::MouseButton::mask(slk::MouseButton::RIGHT) ? IMGUI_MBUT_RIGHT : 0) |
                                       (mouse_buttons_state & slk::MouseButton::mask(slk::MouseButton::MIDDLE) ? IMGUI_MBUT_MIDDLE : 0);

    imguiBeginFrameEx(static_cast<slk::i32>(mouse_pos.x), static_cast<slk::i32>(mouse_pos.y), imgui_button_state,
                      mouse_scroll.y, // wheel status
                      sapp_width(), sapp_height());

    // Main body of the Demo window starts here.
    ImGui::Begin("Slick Debug", nullptr, 0);

    if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Mouse Position: %.2f %.2f", mouse_pos.x, mouse_pos.y);
        ImGui::Text("Mouse button state: %d", mouse_buttons_state);
        ImGui::Text("Mouse buttons state: %d %d %d", slk::InputAPI::isMouseButtonDown(slk::MouseButton::LEFT),
                    slk::InputAPI::isMouseButtonDown(slk::MouseButton::MIDDLE), slk::InputAPI::isMouseButtonDown(slk::MouseButton::RIGHT));
        ImGui::Text("Mouse Scroll: %.2f %.2f", mouse_scroll.x, mouse_scroll.y);

        ImGui::Text("Keyboard SPACE key state: %d", slk::InputAPI::isKeyboardKeyDown(slk::EVirtualKey::SPACE));


        ImGui::Text("Modifiers State:");
        ImGui::Text("   CTRL: %d", slk::InputAPI::hasModifier(slk::InputModifier::CTRL));
        ImGui::Text("   SHIFT %d", slk::InputAPI::hasModifier(slk::InputModifier::SHIFT));
        ImGui::Text("   ALT %d", slk::InputAPI::hasModifier(slk::InputModifier::ALT));
        ImGui::Text("   LMB: %d", slk::InputAPI::hasModifier(slk::InputModifier::LMB));
        ImGui::Text("   MMB %d", slk::InputAPI::hasModifier(slk::InputModifier::MMB));
        ImGui::Text("   RMB %d", slk::InputAPI::hasModifier(slk::InputModifier::RMB));
    }

    ImGui::End();

    imguiEndFrame();
}

void app_frame(void) {
    slk::InputAPI::update();

    if (slk::InputAPI::isKeyboardKeyDown(slk::EVirtualKey::ESCAPE)) {
        sapp_request_quit();
        return;
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xDDDDDDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, sapp_width(), sapp_height());

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    slk::Matrix4f view, proj;
    const slk::Vector3f at = {0.0f, 0.0f, 0.0f};
    const slk::Vector3f eye = {10.0f, 10.0f, 10.0f};
    view.setLookAt(eye, at, slk::Vector3f::UNITY);
    proj.setPerspectiveProj(45.0_deg, sapp_widthf() / sapp_heightf(), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view.data(), proj.data());

    // submit cube
    {
        bgfx::setVertexBuffer(0, g_state.vert_buff_hdl);
        bgfx::setIndexBuffer(g_state.idx_buff_hdl);

        bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z);

        bgfx::submit(0, g_state.prg_hdl);
    }

    // sumbit grid
    {
        // float grid_half_size[4] = {20.f, 0.f, 0.f, 0.f};
        float thin_color[4] = {161 / 255.f, 161 / 255.f, 161 / 255.f, 1.0f};
        float thick_color[4] = {83 / 255.f, 83 / 255.f, 83 / 255.f, 1.0f};
        float grid_scalar_params[4] = {
            4000.f, // grid half size
            1.f, // cell size
            0.f, // dbg viz
            0.f, // fade mode
        };

        bgfx::setTransform(slk::Matrix4f::IDENTITY.data());

        bgfx::setUniform(g_state.grid.grid_scalar_params_u_hdl, grid_scalar_params);
        bgfx::setUniform(g_state.grid.thin_color_u_hdl, thin_color);
        bgfx::setUniform(g_state.grid.thick_color_u_hdl, thick_color);

        bgfx::setVertexBuffer(0, g_state.grid.vert_buff_hdl);
        bgfx::setIndexBuffer(g_state.grid.idx_buff_hdl);

        const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA;
        bgfx::setState(state);

        bgfx::submit(0, g_state.grid.prg_hdl);
    }

    renderSlickWindow();

    bgfx::frame();
}

void app_cleanup(void) {
    slk::InputAPI::shutdown();

    imguiDestroy();

    bgfx::destroy(g_state.prg_hdl);
    bgfx::destroy(g_state.vert_buff_hdl);
    bgfx::destroy(g_state.idx_buff_hdl);

    bgfx::destroy(g_state.grid.prg_hdl);
    bgfx::destroy(g_state.grid.vert_buff_hdl);
    bgfx::destroy(g_state.grid.idx_buff_hdl);

    bgfx::destroy(g_state.grid.grid_scalar_params_u_hdl);
    bgfx::destroy(g_state.grid.thin_color_u_hdl);
    bgfx::destroy(g_state.grid.thick_color_u_hdl);

    bgfx::shutdown();
}

static_assert(SAPP_MOUSEBUTTON_LEFT == (slk::i32)slk::MouseButton::LEFT);
static_assert(SAPP_MOUSEBUTTON_RIGHT == (slk::i32)slk::MouseButton::RIGHT);
static_assert(SAPP_MOUSEBUTTON_MIDDLE == (slk::i32)slk::MouseButton::MIDDLE);

// TODO: modifiers
// TODO: Key Char
void app_event(const sapp_event* evt) {
    switch (evt->type) {
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            slk::MouseEvent slk_event;
            slk_event.type = slk::InputEvent::MOUSE_MOVE;
            slk_event.postion = slk::Vector2f{evt->mouse_x, evt->mouse_y};

            slk::InputAPI::forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            slk::MouseEvent slk_event;
            slk_event.type = slk::InputEvent::MOUSE_BUTTON_DOWN;
            slk_event.button = static_cast<slk::EMouseButton>(evt->mouse_button);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            slk::MouseEvent slk_event;
            slk_event.type = slk::InputEvent::MOUSE_BUTTON_UP;
            slk_event.button = static_cast<slk::EMouseButton>(evt->mouse_button);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            slk::MouseEvent slk_event;
            slk_event.type = slk::InputEvent::MOUSE_SCROLL;
            slk_event.scroll = {evt->scroll_x, evt->scroll_y};
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        // sapp_keycode key_code;              // the virtual key code, only valid in KEY_UP, KEY_DOWN
        // uint32_t char_code;                 // the UTF-32 character code, only valid in CHAR events
        // bool key_repeat;                    // true if this is a key-repeat event, valid in KEY_UP, KEY_DOWN and CHAR
        case SAPP_EVENTTYPE_KEY_DOWN: {
            slk::KeyboardEvent slk_event;
            slk_event.type = slk::InputEvent::KEYBOARD_KEY_DOWN;
            slk_event.key_repeat = evt->key_repeat;
            slk_event.vkey = static_cast<slk::EVirtualKey>(evt->key_code);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            slk::KeyboardEvent slk_event;
            slk_event.type = slk::InputEvent::KEYBOARD_KEY_UP;
            slk_event.key_repeat = evt->key_repeat;
            slk_event.vkey = static_cast<slk::EVirtualKey>(evt->key_code);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        // Note: Used for text input e.g. 'A' == VirtualKey::CHAR_A + InputModifier::SHIFT 
        // SAPP_EVENTTYPE_CHAR
        // Not needed atm
        // SAPP_EVENTTYPE_MOUSE_ENTER

        // unprocessed evts:
        // - SAPP_EVENTTYPE_MOUSE_ENTER,
        // - SAPP_EVENTTYPE_MOUSE_LEAVE,
        default: {
        }
    }
}
