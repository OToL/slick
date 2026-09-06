#include "app.hpp"

#include <slk/math/vector3.hpp>
#include <slk/math/matrix2.hpp>
#include <slk/math/matrix3.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/math/graphics.hpp>
#include <slk/math/utils.hpp>
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
#include <chrono>

import std;

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
    {{-1.f, 1.f, 1.f}, slk::ColorU32::black().m_abgr},
    {{1.f, 1.f, 1.f}, slk::ColorU32::red().m_abgr},
    {{-1.f, -1.f, 1.f}, slk::ColorU32::green().m_abgr},
    {{1.f, -1.f, 1.f}, 0xFF00FFFFU},
    {{-1.f, 1.f, -1.f}, slk::ColorU32::blue().m_abgr},
    {{1.f, 1.f, -1.f}, 0xFFFF00FFU},
    {{-1.f, -1.f, -1.f}, 0xFFFFFF00U},
    {{1.f, -1.f, -1.f}, slk::ColorU32::white().m_abgr},
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
    bgfx::VertexBufferHandle m_vert_buff_hdl;
    bgfx::IndexBufferHandle m_idx_buff_hdl;
    bgfx::VertexLayout m_vert_decl;
    bgfx::ProgramHandle m_prg_hdl;

    bgfx::UniformHandle m_grid_scalar_params_u_hdl;
    bgfx::UniformHandle m_thin_color_u_hdl;
    bgfx::UniformHandle m_thick_color_u_hdl;
};

struct InputSettings {
    slk::f32 m_camera_rotate_speed;
    slk::f32 m_camera_translate_speed;
};

struct AppState {
    using Timer = std::chrono::high_resolution_clock;

    bgfx::VertexBufferHandle m_vert_buff_hdl;
    bgfx::IndexBufferHandle m_idx_buff_hdl;
    bgfx::ProgramHandle m_prg_hdl;
    bgfx::VertexLayout m_vert_decl;

    Grid m_grid;
    slk::Camera m_camera;

    InputSettings m_input_settings;

    Timer::time_point m_last_frame_time_point;
    slk::f32 m_last_frame_time_ms;

    slk::b8 m_imgui_wnd_focused;
} g_app_state;

const slk::f32 CAMERA_DEFAULT_ROTATE_SPEED = 0.0008f;
const slk::f32 CAMERA_DEFAULT_TRANSLATE_SPEED = 0.8f;

void app_init(void) {
    g_app_state = {};

    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count;
    bgfx_init.resolution.width = sapp_width();
    bgfx_init.resolution.height = sapp_height();
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData.nwh = app_get_window_hdl();

    bgfx::init(bgfx_init);
    bgfx::ShaderHandle vs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/vs_cubes.bin");
    bgfx::ShaderHandle fs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/fs_cubes.bin");
    g_app_state.m_prg_hdl = bgfx::createProgram(vs_hdl, fs_hdl, true);

    assert(isValid(vs_hdl) && isValid(fs_hdl) && isValid(g_app_state.m_prg_hdl));

    g_app_state.m_input_settings.m_camera_rotate_speed = CAMERA_DEFAULT_ROTATE_SPEED;
    g_app_state.m_input_settings.m_camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED;

    g_app_state.m_vert_decl.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    g_app_state.m_vert_buff_hdl = bgfx::createVertexBuffer(bgfx::makeRef(CUBE_VERTICES, sizeof(CUBE_VERTICES)), g_app_state.m_vert_decl);
    g_app_state.m_idx_buff_hdl = bgfx::createIndexBuffer(bgfx::makeRef(CUBE_INDICES, sizeof(CUBE_INDICES)));
    assert(isValid(g_app_state.m_vert_buff_hdl) && isValid(g_app_state.m_idx_buff_hdl));

    Grid grid;
    grid.m_vert_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();

    grid.m_vert_buff_hdl = bgfx::createVertexBuffer(bgfx::makeRef(GRID_VERTICES, sizeof(GRID_VERTICES)), grid.m_vert_decl);
    grid.m_idx_buff_hdl = bgfx::createIndexBuffer(bgfx::makeRef(GRID_INDICES, sizeof(GRID_INDICES)));
    assert(isValid(grid.m_vert_buff_hdl) && isValid(grid.m_idx_buff_hdl));

    vs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/grid_vs.sc.bin");
    fs_hdl = loadShader("samples/test_bgfx/_build/shaders/metal/grid_fs.sc.bin");
    grid.m_prg_hdl = bgfx::createProgram(vs_hdl, fs_hdl, true);
    assert(isValid(vs_hdl) && isValid(fs_hdl) && isValid(g_app_state.m_prg_hdl));

    grid.m_grid_scalar_params_u_hdl = bgfx::createUniform("u_grid_scalar_params", bgfx::UniformType::Vec4);
    grid.m_thin_color_u_hdl = bgfx::createUniform("u_thin_color", bgfx::UniformType::Vec4);
    grid.m_thick_color_u_hdl = bgfx::createUniform("u_thick_color", bgfx::UniformType::Vec4);

    assert(isValid(grid.m_thin_color_u_hdl) && isValid(grid.m_thick_color_u_hdl) && isValid(grid.m_grid_scalar_params_u_hdl));

    g_app_state.m_grid = grid;

    // slk::Matrix4f view;
    const slk::Vector3f at = {0.0f, 0.0f, 0.0f};
    const slk::Vector3f eye = {10.0f, 10.0f, 0.0f};

    // g_state.camera = {
    //     .eye_pos = {10.f, 10.f, 10.f},
    //     .target_pos = {0.f, 0.f, 0.f}
    // };
    g_app_state.m_camera.setLookAt(eye, at);
    g_app_state.m_camera.setPerspectiveProjection(0.1f, 10000.0f, 45.0_deg, sapp_widthf() / sapp_heightf());

    g_app_state.m_last_frame_time_point = std::chrono::high_resolution_clock::now();
    g_app_state.m_last_frame_time_ms = 0.f;

    imguiCreate();

    slk::InputAPI::initialize();
}

void render_slick_dbg_window() {
    slk::Vector2f const mouse_pos = slk::InputAPI::mousePosition();
    slk::Vector2f const mouse_scroll = slk::InputAPI::mouseScroll();
    slk::MouseButtonMask const mouse_buttons_state = slk::InputAPI::mouseButtonsState();
    slk::u8 const imgui_button_state = ((mouse_buttons_state & slk::MouseButtonMask::LEFT) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_LEFT : 0) |
                                       ((mouse_buttons_state & slk::MouseButtonMask::RIGHT) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_RIGHT : 0) |
                                       ((mouse_buttons_state & slk::MouseButtonMask::MIDDLE) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_MIDDLE : 0);

    imguiBeginFrameEx(static_cast<slk::i32>(mouse_pos.m_x), static_cast<slk::i32>(mouse_pos.m_y), imgui_button_state,
                      mouse_scroll.m_y, // wheel status
                      sapp_width(), sapp_height());

    static bool initialized = false;

    // Main body of the Demo window starts here.
    ImGui::Begin("Slick Debug", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    // Hack because, even with ImGuiWindowFlags_NoFocusOnAppearing, the window is focused
    if (!initialized) {
        ImGui::SetWindowFocus(nullptr); // unfocus all windows
        initialized = true; // set to true once you want to stop doing this
    }

    g_app_state.m_imgui_wnd_focused |= ImGui::IsWindowFocused();

    ImGui::Text("Frame Time: %.2f ms", g_app_state.m_last_frame_time_ms);
    if (ImGui::CollapsingHeader("Inputs", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Mouse Position: %.2f %.2f", mouse_pos.m_x, mouse_pos.m_y);
            ImGui::Text("Mouse button state: %d", mouse_buttons_state);
            ImGui::Text("Mouse buttons state: %d %d %d", slk::InputAPI::isMouseButtonDown(slk::MouseButton::LEFT),
                        slk::InputAPI::isMouseButtonDown(slk::MouseButton::MIDDLE), slk::InputAPI::isMouseButtonDown(slk::MouseButton::RIGHT));
            ImGui::Text("Mouse Scroll: %.2f %.2f", mouse_scroll.m_x, mouse_scroll.m_y);

            ImGui::Text("Keyboard SPACE key state: %d", slk::InputAPI::isKeyboardKeyDown(slk::KeyboardVKey::SPACE));

            ImGui::Text("Modifiers State:");
            ImGui::Text("   CTRL: %d", slk::InputAPI::hasModifier(slk::InputModifier::CTRL));
            ImGui::Text("   SHIFT %d", slk::InputAPI::hasModifier(slk::InputModifier::SHIFT));
            ImGui::Text("   ALT %d", slk::InputAPI::hasModifier(slk::InputModifier::ALT));
            ImGui::Text("   LMB: %d", slk::InputAPI::hasModifier(slk::InputModifier::LMB));
            ImGui::Text("   MMB %d", slk::InputAPI::hasModifier(slk::InputModifier::MMB));
            ImGui::Text("   RMB %d", slk::InputAPI::hasModifier(slk::InputModifier::RMB));

            ImGui::Text("Touch in progress: %d", slk::InputAPI::gestureTouchCount());
        }
        if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {

            static slk::f32 camera_multiplier = 1.f;
            ImGui::SliderFloat("Came", &camera_multiplier, 0.1f, 2.f);

            g_app_state.m_input_settings.m_camera_rotate_speed = CAMERA_DEFAULT_ROTATE_SPEED * camera_multiplier;
            g_app_state.m_input_settings.m_camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED * camera_multiplier;
        }
    }

    ImGui::End();
    imguiEndFrame();
}

void app_frame(void) {
    slk::InputAPI::update();

    if (slk::InputAPI::isKeyboardKeyDown(slk::KeyboardVKey::ESCAPE)) {
        sapp_request_quit();
        return;
    }

    auto const time_point = AppState::Timer::now();
    g_app_state.m_last_frame_time_ms =
        std::chrono::duration_cast<std::chrono::microseconds>(time_point - g_app_state.m_last_frame_time_point).count() / 1000.f;
    g_app_state.m_last_frame_time_point = time_point;

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xDDDDDDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, sapp_width(), sapp_height());

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    // Update camera
    // TODO: Limit rotation to avoid gimbal lock
    if (!g_app_state.m_imgui_wnd_focused) {
        if (slk::InputAPI::gestureTouchCount() == 0)
        {
            slk::Vector2f const mouse_movement = slk::InputAPI::mouseMovement();

            if (slk::InputAPI::areMouseButtonsDown(slk::MouseButtonMask::RIGHT | slk::MouseButtonMask::LEFT) ||
                    slk::InputAPI::isMouseButtonDown(slk::MouseButton::MIDDLE)) {
                if (mouse_movement.m_x != 0.f || mouse_movement.m_y != 0.f) {
                    g_app_state.m_camera.pan(mouse_movement.m_x * CAMERA_DEFAULT_TRANSLATE_SPEED, mouse_movement.m_y * CAMERA_DEFAULT_TRANSLATE_SPEED);
                }

            } else if (slk::InputAPI::isMouseButtonDown(slk::MouseButton::RIGHT)) {
                if ((mouse_movement.m_x != 0.f) || (mouse_movement.m_y != 0.f)) {
                    slk::Vector2f const camera_rot = mouse_movement * CAMERA_DEFAULT_ROTATE_SPEED * g_app_state.m_last_frame_time_ms;
                    g_app_state.m_camera.rotate(camera_rot.m_x, camera_rot.m_y);
                }
            } else if (slk::InputAPI::isMouseButtonDown(slk::MouseButton::LEFT)) {
                if (mouse_movement.m_y != 0.f) {
                    g_app_state.m_camera.translate(slk::Vector3f{0.f, 0.f, mouse_movement.m_y * CAMERA_DEFAULT_TRANSLATE_SPEED});
                }
            }
        }
        // Gesture
        else if (slk::InputAPI::gestureTouchCount() == 2) {
            // g_app_state.m_input_settings.m_camera_rotate_speed = CAMERA_DEFAULT_ROTATE_SPEED;
            // g_app_state.m_input_settings.m_camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED;

            slk::b8 const is_left = slk::InputAPI::isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_A, slk::KeyboardVKey::LEFT});
            slk::b8 const is_right = slk::InputAPI::isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_D, slk::KeyboardVKey::RIGHT});
            slk::b8 const is_up = slk::InputAPI::isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_W, slk::KeyboardVKey::UP});
            slk::b8 const is_down = slk::InputAPI::isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_S, slk::KeyboardVKey::DOWN});

            slk::Vector2f panning{};
            slk::Vector3f translate{};

            panning.m_x -= is_left * g_app_state.m_input_settings.m_camera_translate_speed;
            panning.m_x += is_right * g_app_state.m_input_settings.m_camera_translate_speed;
            translate.m_z -= is_down * g_app_state.m_input_settings.m_camera_translate_speed;
            translate.m_z += is_up * g_app_state.m_input_settings.m_camera_translate_speed;

            g_app_state.m_camera.pan(panning.m_x, panning.m_y);
            g_app_state.m_camera.translate(translate);

            slk::Vector2f const mouse_scroll = slk::InputAPI::gestureScrollState();
            if ((mouse_scroll.m_x != 0.f) || (mouse_scroll.m_y != 0.f)) {
                slk::Vector2f const camera_rot = mouse_scroll * g_app_state.m_input_settings.m_camera_rotate_speed * g_app_state.m_last_frame_time_ms;
                g_app_state.m_camera.rotate(-camera_rot.m_x, -camera_rot.m_y);
            }
        }
    }

    slk::Matrix4f const proj = g_app_state.m_camera.projectionMatrix(slk::Handedness::DEFAULT, bgfx::getCaps()->homogeneousDepth);
    slk::Matrix4f const view = g_app_state.m_camera.viewMatrix();
    bgfx::setViewTransform(0, view.data(), proj.data());

    // submit cube
    {
        bgfx::setVertexBuffer(0, g_app_state.m_vert_buff_hdl);
        bgfx::setIndexBuffer(g_app_state.m_idx_buff_hdl);

        bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z);

        bgfx::submit(0, g_app_state.m_prg_hdl);
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

        bgfx::setUniform(g_app_state.m_grid.m_grid_scalar_params_u_hdl, grid_scalar_params);
        bgfx::setUniform(g_app_state.m_grid.m_thin_color_u_hdl, thin_color);
        bgfx::setUniform(g_app_state.m_grid.m_thick_color_u_hdl, thick_color);

        bgfx::setVertexBuffer(0, g_app_state.m_grid.m_vert_buff_hdl);
        bgfx::setIndexBuffer(g_app_state.m_grid.m_idx_buff_hdl);

        const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA;
        bgfx::setState(state);

        bgfx::submit(0, g_app_state.m_grid.m_prg_hdl);
    }

    g_app_state.m_imgui_wnd_focused = false;
    render_slick_dbg_window();

    bgfx::frame();
}

void app_cleanup(void) {
    slk::InputAPI::shutdown();

    imguiDestroy();

    bgfx::destroy(g_app_state.m_prg_hdl);
    bgfx::destroy(g_app_state.m_vert_buff_hdl);
    bgfx::destroy(g_app_state.m_idx_buff_hdl);

    bgfx::destroy(g_app_state.m_grid.m_prg_hdl);
    bgfx::destroy(g_app_state.m_grid.m_vert_buff_hdl);
    bgfx::destroy(g_app_state.m_grid.m_idx_buff_hdl);

    bgfx::destroy(g_app_state.m_grid.m_grid_scalar_params_u_hdl);
    bgfx::destroy(g_app_state.m_grid.m_thin_color_u_hdl);
    bgfx::destroy(g_app_state.m_grid.m_thick_color_u_hdl);

    bgfx::shutdown();
}

static_assert(SAPP_MOUSEBUTTON_LEFT == std::to_underlying(slk::MouseButton::LEFT));
static_assert(SAPP_MOUSEBUTTON_RIGHT == std::to_underlying(slk::MouseButton::RIGHT));
static_assert(SAPP_MOUSEBUTTON_MIDDLE == std::to_underlying(slk::MouseButton::MIDDLE));

void app_notify_gesture_touch_count(int count) {
    slk::GestureEvent slk_event;
    slk_event.m_type = slk::InputEventType::GESTURE_TOUCH_COUNT_UPDATE;
    slk_event.m_point_count = count;
    slk::InputAPI::forwardEvent(slk_event);
}

void app_notify_gesture_scroll(int touch_count, float deltax, float deltay) {

    slk::GestureEvent slk_event;
    slk_event.m_type = slk::InputEventType::GESTURE_SCROLL;
    slk_event.m_point_count = touch_count;
    slk_event.m_delta = {deltax, deltay};
    slk::InputAPI::forwardEvent(slk_event);
}

// TODO: Key Char
void app_event(const sapp_event* evt) {
    switch (evt->type) {
        // SAPP_EVENTTYPE_TOUCHES_MOVED,
        /* case SAPP_EVENTTYPE_TOUCHES_BEGAN:
            {
                g_dbg_touch_inprogress = true;
                break;
            }
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
            [[fallthrough]];
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
            {
                g_dbg_touch_inprogress = false;
                break;
            } */
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_MOVE;
            slk_event.m_postion = slk::Vector2f{evt->mouse_x, evt->mouse_y};

            slk::InputAPI::forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_BUTTON_DOWN;
            slk_event.m_button = static_cast<slk::MouseButton>(evt->mouse_button);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_BUTTON_UP;
            slk_event.m_button = static_cast<slk::MouseButton>(evt->mouse_button);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_SCROLL;
            slk_event.m_scroll = {evt->scroll_x, evt->scroll_y};
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        // sapp_keycode key_code;              // the virtual key code, only valid in KEY_UP, KEY_DOWN
        // uint32_t char_code;                 // the UTF-32 character code, only valid in CHAR events
        // bool key_repeat;                    // true if this is a key-repeat event, valid in KEY_UP, KEY_DOWN and CHAR
        case SAPP_EVENTTYPE_KEY_DOWN: {
            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::InputEventType::KEYBOARD_KEY_DOWN;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::KeyboardVKey>(evt->key_code);
            slk::InputAPI::forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::InputEventType::KEYBOARD_KEY_UP;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::KeyboardVKey>(evt->key_code);
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
