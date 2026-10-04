#include "graphics.hpp"
#include "demos.hpp"
#include "demo_empty.hpp"
#include "demo_curves.hpp"
#include "app.hpp"

#include <slk/color.hpp>
#include <slk/camera.hpp>
#include <slk/math/utils.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/input/input_api.hpp>
#include <slk/input/events.hpp>

#include <sokol/sokol_app.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bgfx_utils/imgui/imgui.h>

#include <cassert>
#include <chrono>

#include <bgfx_utils/debugdraw/debugdraw.h>

import std;

using namespace slk::literals;

constexpr slk::f32 CAMERA_DEFAULT_ROTATE_SPEED = 0.0008f;
constexpr slk::f32 CAMERA_DEFAULT_TRANSLATE_SPEED = 0.8f;

struct InputSettings {
    slk::f32 camera_rotate_speed;
    slk::f32 camera_translate_speed;
};

struct AppState {
    using Timer = std::chrono::high_resolution_clock;

    Demos demos;

    InputSettings input_settings;

    slk::Camera camera;
    GridHdl grid_hdl;

    Timer::time_point last_frame_time_point;
    slk::f32 last_frame_time_ms;

    slk::b8 imgui_wnd_focused = false;

} g_app_state{};

void renderDbgWnd(AppState& app_state) {
    slk::InputApi const* const input = slk::InputApi::instance();
    slk::Vector2f const mouse_pos = input->mousePosition();
    slk::Vector2f const mouse_scroll = input->mouseScroll();
    slk::MouseButtonMask const mouse_buttons_state = input->mouseButtonsState();
    slk::u8 const imgui_button_state = ((mouse_buttons_state & slk::MouseButtonMask::LEFT) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_LEFT : 0) |
                                       ((mouse_buttons_state & slk::MouseButtonMask::RIGHT) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_RIGHT : 0) |
                                       ((mouse_buttons_state & slk::MouseButtonMask::MIDDLE) != slk::MouseButtonMask::NONE ? IMGUI_MBUT_MIDDLE : 0);

    imguiBeginFrameEx(static_cast<slk::i32>(mouse_pos.m_x), static_cast<slk::i32>(mouse_pos.m_y), imgui_button_state,
                      mouse_scroll.m_y, // wheel status
                      sapp_width(), sapp_height());

    static bool initialized = false;

    // Hack because, even with ImGuiWindowFlags_NoFocusOnAppearing, the window is focused
    if (!initialized) {
        ImGui::SetWindowFocus(nullptr); // unfocus all windows
        initialized = true; // set to true once you want to stop doing this
    }

    // Main body of the Demo window starts here.
    ImGui::SetNextWindowSize(ImVec2(250, 380), ImGuiCond_FirstUseEver);

    ImGui::Begin("Slick Demos", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    // Must be called after ImGui::Begin because it is related to the currently processed window
    app_state.imgui_wnd_focused = ImGui::IsWindowFocused();

    ImGui::Text("Frame Time: %.2f ms", g_app_state.last_frame_time_ms);

    auto const demos = g_app_state.demos.demos();
    auto const curr_demo_id = g_app_state.demos.currentId();
    auto const curr_demo_it = std::ranges::find_if(demos, [curr_demo_id](DemoDesc const& desc) { return strcmp(curr_demo_id, desc.id) == 0; });
    assert(curr_demo_it != demos.end());

    auto const dbg_demo_raw_names = demos | 
        std::views::transform([](DemoDesc const& desc) {return desc.info.name.data();}) | 
        std::ranges::to<std::vector<char const*>>();

    slk::i32 const curr_demo_idx = std::distance(demos.begin(), curr_demo_it);
    slk::i32 next_demo_idx = curr_demo_idx;

    ImGui::TextUnformatted("Demo:"); ImGui::SameLine(); ImGui::Combo("##DemoSelection", &next_demo_idx, dbg_demo_raw_names.data(), dbg_demo_raw_names.size());

    if (ImGui::CollapsingHeader("General")) {
        static slk::b8 show_imgui_demo = false;
        ImGui::Checkbox("Show ImGui demo", &show_imgui_demo);
        if (show_imgui_demo)
            ImGui::ShowDemoWindow(&show_imgui_demo);

        ImGui::Spacing(); ImGui::Spacing();

        ImGui::TextUnformatted("Free camera default controls:");
        ImGui::TextUnformatted("- Mouse Wheel to Zoom in-out");
        ImGui::TextUnformatted("- Mouse Wheel Pressed to Pan");
        ImGui::TextUnformatted("- Z to zoom to (0, 0, 0)");

        ImGui::Text("Mouse Position: %.2f %.2f", mouse_pos.m_x, mouse_pos.m_y);
        ImGui::Text("Mouse button state: %d", mouse_buttons_state);
        ImGui::Text("Mouse buttons state: %d %d %d", input->isMouseButtonDown(slk::MouseButton::LEFT),
                    input->isMouseButtonDown(slk::MouseButton::MIDDLE), input->isMouseButtonDown(slk::MouseButton::RIGHT));
        ImGui::Text("Mouse Scroll: %.2f %.2f", mouse_scroll.m_x, mouse_scroll.m_y);

        ImGui::Text("Keyboard SPACE key state: %d", input->isKeyboardKeyDown(slk::KeyboardVKey::SPACE));

        ImGui::Text("Modifiers State:");
        ImGui::Text("   CTRL: %d", input->hasModifier(slk::InputModifier::CTRL));
        ImGui::Text("   SHIFT %d", input->hasModifier(slk::InputModifier::SHIFT));
        ImGui::Text("   ALT %d", input->hasModifier(slk::InputModifier::ALT));
        ImGui::Text("   LMB: %d", input->hasModifier(slk::InputModifier::LMB));
        ImGui::Text("   MMB %d", input->hasModifier(slk::InputModifier::MMB));
        ImGui::Text("   RMB %d", input->hasModifier(slk::InputModifier::RMB));

        ImGui::Text("Touch in progress: %d", input->gestureTouchCount());

    }

    if (next_demo_idx != curr_demo_idx)
    {
        g_app_state.demos.setCurrent(demos[next_demo_idx].id);
    }

    DemoDesc const curr_demo_desc = app_state.demos.currentDesc();
    if (allMaskFlagSet(curr_demo_desc.info.caps, DemoCapsMask::DEBUG_IMGUI))
    {
        if (ImGui::CollapsingHeader("Demo", ImGuiTreeNodeFlags_DefaultOpen)) {
            app_state.demos.drawImgui(app_state.last_frame_time_ms);
        }
    }


    ImGui::End();

    imguiEndFrame();
}

// TODO: Limit rotation to avoid gimbal lock
void updateCamera(AppState& app_state, slk::Camera& cam, slk::InputApi const& input) {
    if (input.gestureTouchCount() == 0) {
        slk::Vector2f const mouse_movement = input.mouseMovement();

        if (input.areMouseButtonsDown(slk::MouseButtonMask::RIGHT | slk::MouseButtonMask::LEFT) ||
            input.isMouseButtonDown(slk::MouseButton::MIDDLE)) {
            if (mouse_movement.m_x != 0.f || mouse_movement.m_y != 0.f) {
                cam.pan(mouse_movement.m_x * CAMERA_DEFAULT_TRANSLATE_SPEED, mouse_movement.m_y * CAMERA_DEFAULT_TRANSLATE_SPEED);
            }

        } else if (input.isMouseButtonDown(slk::MouseButton::RIGHT)) {
            if ((mouse_movement.m_x != 0.f) || (mouse_movement.m_y != 0.f)) {
                slk::Vector2f const camera_rot = mouse_movement * CAMERA_DEFAULT_ROTATE_SPEED * g_app_state.last_frame_time_ms;
                cam.rotate(camera_rot.m_x, camera_rot.m_y);
            }
        } else if (input.isMouseButtonDown(slk::MouseButton::LEFT)) {
            if (mouse_movement.m_y != 0.f) {
                cam.translate(slk::Vector3f{0.f, 0.f, mouse_movement.m_y * CAMERA_DEFAULT_TRANSLATE_SPEED});
            }
        }
    }
    // Gesture
    else if (input.gestureTouchCount() == 2) {
        // g_app_state.m_input_settings.camera_rotate_speed = CAMERA_DEFAULT_ROTATE_SPEED;
        // g_app_state.m_input_settings.camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED;

        slk::b8 const is_left = input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_A, slk::KeyboardVKey::LEFT});
        slk::b8 const is_right = input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_D, slk::KeyboardVKey::RIGHT});
        slk::b8 const is_up = input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_W, slk::KeyboardVKey::UP});
        slk::b8 const is_down = input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_S, slk::KeyboardVKey::DOWN});

        slk::Vector2f panning{};
        slk::Vector3f translate{};

        panning.m_x -= is_left * app_state.input_settings.camera_translate_speed;
        panning.m_x += is_right * app_state.input_settings.camera_translate_speed;
        translate.m_z -= is_down * app_state.input_settings.camera_translate_speed;
        translate.m_z += is_up * app_state.input_settings.camera_translate_speed;

        cam.pan(panning.m_x, panning.m_y);
        cam.translate(translate);

        slk::Vector2f const mouse_scroll = input.gestureScrollState();
        if ((mouse_scroll.m_x != 0.f) || (mouse_scroll.m_y != 0.f)) {
            slk::Vector2f const camera_rot = mouse_scroll * g_app_state.input_settings.camera_rotate_speed * g_app_state.last_frame_time_ms;
            cam.rotate(-camera_rot.m_x, -camera_rot.m_y);
        }
    }
}

void appInitialize() {
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count;
    bgfx_init.resolution.width = sapp_width();
    bgfx_init.resolution.height = sapp_height();
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData.nwh = appGetWindowHdl();

    bgfx::init(bgfx_init);
    // bgfx::setDebug(BGFX_DEBUG_STATS);

    g_app_state.grid_hdl = createGrid();
    assert(g_app_state.grid_hdl);

    imguiCreate();

    slk::InputApi::initialize();

    g_app_state.camera.setLookAt({10.0f, 10.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
    g_app_state.camera.setPerspectiveProjection(0.1f, 10000.0f, 45.0_deg, sapp_widthf() / sapp_heightf());

    g_app_state.input_settings.camera_rotate_speed = CAMERA_DEFAULT_ROTATE_SPEED;
    g_app_state.input_settings.camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED;

    g_app_state.last_frame_time_point = std::chrono::high_resolution_clock::now();
    g_app_state.last_frame_time_ms = 0.f;

    g_app_state.demos.initialize({
            emptyDemo(),
            curvesDemo()});
    g_app_state.demos.setCurrent(EMPTY_DEMO_ID);

    // debug draw init
    ddInit();
}

void appUpdate() {
    constexpr slk::ColorU32 THIN_COLOR{255, 161, 161, 161};
    constexpr slk::ColorU32 THICK_COLOR{255, 83, 83, 83};

    slk::InputApi* const input = slk::InputApi::instance();

    input->update();

    if (input->isKeyboardKeyDown(slk::KeyboardVKey::ESCAPE)) {
        sapp_request_quit();
        return;
    }

    auto const time_point = AppState::Timer::now();
    g_app_state.last_frame_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(time_point - g_app_state.last_frame_time_point).count() / 1000.f;
    g_app_state.last_frame_time_point = time_point;

    if (!g_app_state.imgui_wnd_focused)
        updateCamera(g_app_state, g_app_state.camera, *input);

    if (!g_app_state.demos.update(g_app_state.last_frame_time_ms, g_app_state.camera))
    {
        sapp_request_quit();
        return;
    }

    slk::Matrix4f const proj = g_app_state.camera.projectionMatrix(slk::Handedness::DEFAULT, bgfx::getCaps()->homogeneousDepth);
    slk::Matrix4f const view = g_app_state.camera.viewMatrix();

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xDDDDDDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, sapp_width(), sapp_height());

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    bgfx::setViewTransform(0, view.data(), proj.data());


    renderDbgWnd(g_app_state);

    g_app_state.demos.draw3d(g_app_state.last_frame_time_ms, g_app_state.camera);

    renderGrid(g_app_state.grid_hdl, THIN_COLOR, THICK_COLOR, true);

    g_app_state.demos.draw2d(g_app_state.last_frame_time_ms);


    // TEMP DEBUG: dump the actual rendered backbuffer to disk for direct inspection
    static int frame_count = 0;
    ++frame_count;
    if (frame_count == 30) {
        bgfx::requestScreenShot(BGFX_INVALID_HANDLE, "/tmp/graphics_sandbox_frame");
    }

    bgfx::frame();
}

void appShutdown() {

    ddShutdown();

    g_app_state.demos.shutdown();

    imguiDestroy();

    destroyGrid(g_app_state.grid_hdl);

    bgfx::shutdown();

    slk::InputApi::shutdown();

    g_app_state = {};
}

void appNotifyEvent(const sapp_event* evt) {
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

            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_BUTTON_DOWN;
            slk_event.m_button = static_cast<slk::MouseButton>(evt->mouse_button);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_BUTTON_UP;
            slk_event.m_button = static_cast<slk::MouseButton>(evt->mouse_button);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::InputEventType::MOUSE_SCROLL;
            slk_event.m_scroll = {evt->scroll_x, evt->scroll_y};
            slk::InputApi::instance()->forwardEvent(slk_event);

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
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::InputEventType::KEYBOARD_KEY_UP;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::KeyboardVKey>(evt->key_code);
            slk::InputApi::instance()->forwardEvent(slk_event);

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

void appNotifyGestureTouchCount(int count) {
    slk::GestureEvent slk_event;
    slk_event.m_type = slk::InputEventType::GESTURE_TOUCH_COUNT_UPDATE;
    slk_event.m_point_count = count;
    slk::InputApi::instance()->forwardEvent(slk_event);
}

void appNotifyGestureScroll(int touch_count, float deltax, float deltay) {
    slk::GestureEvent slk_event;
    slk_event.m_type = slk::InputEventType::GESTURE_SCROLL;
    slk_event.m_point_count = touch_count;
    slk_event.m_delta = {deltax, deltay};
    slk::InputApi::instance()->forwardEvent(slk_event);
}

