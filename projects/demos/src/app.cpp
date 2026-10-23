#include "graphics.hpp"
#include "demos.hpp"
#include "demo_empty.hpp"
#include "demo_curves.hpp"
#include "app.hpp"

#include <slk/color.hpp>
#include <slk/camera.hpp>
#include <slk/math/utils.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/math/graphics.hpp>
#include <slk/math/ray3.hpp>
#include <slk/math/plane.hpp>
#include <slk/math/intersection.hpp>
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

constexpr slk::f32 CAMERA_DEFAULT_ROTATE_SPEED = 0.0003f;
constexpr slk::f32 CAMERA_DEFAULT_TRANSLATE_SPEED = 0.15f;
constexpr slk::Vector3f CAMERA_RESET_POSITION = {10.0f, 10.0f, 0.0f};
constexpr slk::Vector3f CAMERA_RESET_TARGET = {.0f, .0f, .0f};
constexpr slk::f32 CAMERA_MAX_PITCH = 78.5_deg;

struct InputSettings {
    slk::f32 camera_rotation_speed;
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
    slk::b8 orbitting = false;
    slk::Vector3f orbit_pivot = {};
} g_app_state{};

void renderDbgWnd(AppState& app_state) {
    slk::InputApi const* const input = slk::InputApi::instance();
    slk::Vector2f const mouse_pos = input->mousePosition();
    slk::Vector2f const mouse_scroll = input->mouseScroll();
    slk::EMouseButtonMask const mouse_buttons_state = input->mouseButtonsState();
    slk::u8 const imgui_button_state = ((mouse_buttons_state & slk::EMouseButtonMask::LEFT) != slk::EMouseButtonMask::NONE ? IMGUI_MBUT_LEFT : 0) |
                                       ((mouse_buttons_state & slk::EMouseButtonMask::RIGHT) != slk::EMouseButtonMask::NONE ? IMGUI_MBUT_RIGHT : 0) |
                                       ((mouse_buttons_state & slk::EMouseButtonMask::MIDDLE) != slk::EMouseButtonMask::NONE ? IMGUI_MBUT_MIDDLE : 0);

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
    ImGui::SetNextWindowSize(ImVec2(280, 380), ImGuiCond_FirstUseEver);

    ImGui::Begin("Slick Demos", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    // Must be called after ImGui::Begin because it is related to the currently processed window
    // app_state.imgui_wnd_focused = ImGui::IsWindowFocused();

    ImGui::Text("Frame Time: %.2f ms", g_app_state.last_frame_time_ms);

    auto const demos = g_app_state.demos.demos();
    auto const curr_demo_id = g_app_state.demos.currentId();
    auto const curr_demo_it = std::ranges::find_if(demos, [curr_demo_id](DemoDesc const& desc) { return strcmp(curr_demo_id, desc.id) == 0; });
    assert(curr_demo_it != demos.end());

    auto const dbg_demo_raw_names =
        demos | std::views::transform([](DemoDesc const& desc) { return desc.info.name.data(); }) | std::ranges::to<std::vector<char const*>>();

    slk::i32 const curr_demo_idx = std::distance(demos.begin(), curr_demo_it);
    slk::i32 next_demo_idx = curr_demo_idx;

    ImGui::PushItemWidth(ImGui::GetFontSize() * 8.f);
    ImGui::TextUnformatted("Demo:");
    ImGui::SameLine();
    ImGui::Combo("##DemoSelection", &next_demo_idx, dbg_demo_raw_names.data(), dbg_demo_raw_names.size());
    ImGui::PopItemWidth();

    if (ImGui::CollapsingHeader("Debug Info")) {
        static slk::b8 show_imgui_demo = false;
        ImGui::Checkbox("Show ImGui demo", &show_imgui_demo);
        if (show_imgui_demo)
            ImGui::ShowDemoWindow(&show_imgui_demo);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::TextUnformatted("Free camera default controls:");
        ImGui::TextUnformatted("- Mouse Wheel to Zoom in-out");
        ImGui::TextUnformatted("- Mouse Wheel Pressed to Pan");
        ImGui::TextUnformatted("- Z to zoom to (0, 0, 0)");

        ImGui::Text("Mouse Position: %.2f %.2f", mouse_pos.m_x, mouse_pos.m_y);
        ImGui::Text("Mouse button state: %d", mouse_buttons_state);
        ImGui::Text("Mouse buttons state: %d %d %d", input->isMouseButtonDown(slk::EMouseButton::LEFT),
                    input->isMouseButtonDown(slk::EMouseButton::MIDDLE), input->isMouseButtonDown(slk::EMouseButton::RIGHT));
        ImGui::Text("Mouse Scroll: %.2f %.2f", mouse_scroll.m_x, mouse_scroll.m_y);

        ImGui::Text("Keyboard SPACE key state: %d", input->isKeyboardKeyDown(slk::EKeyboardVKey::SPACE));

        ImGui::Text("Modifiers State:");
        ImGui::Text("   CTRL: %d", input->hasModifier(slk::EInputModifier::CTRL));
        ImGui::Text("   SHIFT %d", input->hasModifier(slk::EInputModifier::SHIFT));
        ImGui::Text("   ALT %d", input->hasModifier(slk::EInputModifier::ALT));
        ImGui::Text("   LMB: %d", input->hasModifier(slk::EInputModifier::LMB));
        ImGui::Text("   MMB %d", input->hasModifier(slk::EInputModifier::MMB));
        ImGui::Text("   RMB %d", input->hasModifier(slk::EInputModifier::RMB));

        ImGui::Text("Touch in progress: %d", input->gestureTouchCount());
    }
    if (ImGui::CollapsingHeader("Settings")) {
        ImGui::PushItemWidth(ImGui::GetFontSize() * 8.f);
        ImGui::InputFloat("Translation Speed", &app_state.input_settings.camera_translate_speed, 0.001f, 1.0f, "%.6f");
        ImGui::InputFloat("Rotation Speed ", &app_state.input_settings.camera_rotation_speed, 0.001f, 1.0f, "%.6f");
        ImGui::PopItemWidth();
    }

    if (next_demo_idx != curr_demo_idx) {
        g_app_state.demos.setCurrent(demos[next_demo_idx].id);
    }

    DemoDesc const curr_demo_desc = app_state.demos.currentDesc();
    if (allMaskFlagSet(curr_demo_desc.info.caps, EDemoCapsMask::DEBUG_IMGUI)) {
        if (ImGui::CollapsingHeader("Demo", ImGuiTreeNodeFlags_DefaultOpen)) {
            app_state.demos.drawImgui(app_state.last_frame_time_ms);
        }
    }

    app_state.imgui_wnd_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

    ImGui::End();

    imguiEndFrame();
}

void updateCamera(AppState& app_state, slk::Camera& cam, slk::InputApi const& input) {
    // Frame dependent movement delta
    slk::f32 const translate_frame_factor = app_state.input_settings.camera_translate_speed * app_state.last_frame_time_ms;
    slk::f32 const rotate_frame_factor = app_state.input_settings.camera_rotation_speed * app_state.last_frame_time_ms;

    slk::Vector2f const mouse_scroll = input.mouseScroll();
    slk::Vector2f const gesture_movement = input.mouseMovement();
    slk::Vector2f const gesture_magnify = input.gestureMagnify();
    slk::Vector2f const gesture_scroll = input.gestureScroll();

    slk::b8 const was_orbitting = g_app_state.orbitting;
    g_app_state.orbitting = false;

    // Panning
    if (!was_orbitting &&
        (input.isMouseButtonDown(slk::EMouseButton::MIDDLE) || (input.gestureTouchCount() == 3) ||
         ((input.isMouseButtonDown(slk::EMouseButton::RIGHT) || input.gestureTouchCount() == 2) && input.hasModifier(slk::EInputModifier::SHIFT)))) {
        slk::Vector2f pan_value{};
        if (input.gestureTouchCount() == 2)
            pan_value = gesture_scroll * 0.1f * translate_frame_factor;
        else if (input.gestureTouchCount() == 3)
            pan_value = gesture_scroll * 0.4f * translate_frame_factor;
        else
            pan_value = gesture_movement * 0.02f * translate_frame_factor;
        cam.pan(pan_value.m_x, -pan_value.m_y);
    }
    // 2 fingers & right mouse click basic movements
    else if ((input.gestureTouchCount() == 2) || input.isMouseButtonDown(slk::EMouseButton::RIGHT)) {
        slk::Vector2f panning{};
        slk::Vector3f translate{};
        slk::Vector2f rotation{};

        slk::b8 block_rotation = false;

        if (!was_orbitting) {
            // WASD and Arrows move
            slk::b8 const is_left = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_A, slk::EKeyboardVKey::LEFT});
            slk::b8 const is_right = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_D, slk::EKeyboardVKey::RIGHT});
            slk::b8 const is_up = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_W, slk::EKeyboardVKey::UP});
            slk::b8 const is_down = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_S, slk::EKeyboardVKey::DOWN});

            panning.m_x -= is_left * translate_frame_factor;
            panning.m_x += is_right * translate_frame_factor;
            translate.m_z -= is_down * translate_frame_factor;
            translate.m_z += is_up * translate_frame_factor;

            // Zoom via magnify
            if (gesture_magnify.m_x != 0.f) {
                translate.m_z += gesture_magnify.m_x * translate_frame_factor;
                block_rotation = true;
            }
            // Zoom
            else if (input.hasModifier(slk::EInputModifier::CTRL)) {
                block_rotation = true;
                if (input.gestureTouchCount() == 2)
                    translate.m_z += gesture_scroll.m_y * 0.02f * translate_frame_factor;
                else
                    translate.m_z += gesture_movement.m_y * 0.02f * translate_frame_factor;
            }
        }

        // Rotation
        if (!block_rotation) {
            slk::Vector2f mouse_ndc = slk::screenPosToNdc(input.mousePosition(), slk::Vector2f{sapp_widthf(), sapp_heightf()});
            slk::Ray3f const ray = cam.rayFromNdc(mouse_ndc);

            if (input.gestureTouchCount() == 2)
                rotation = gesture_scroll * rotate_frame_factor;
            else
                rotation = gesture_movement * rotate_frame_factor;

            slk::Matrix3f const prev_cam_orient = cam.m_orientation;

            if ((rotation.m_x != 0.f) || (rotation.m_y != 0.f)) {
                // Only apply the part of the pitch change that stays within the limit, yaw is never limited
                // rotate() applies setRotationX(-delta_pitch) so the pitch after the call is pitch() - delta_pitch
                slk::f32 const pitch = cam.pitch();
                slk::f32 const delta_pitch = pitch - std::clamp(pitch + rotation.m_y, -CAMERA_MAX_PITCH, CAMERA_MAX_PITCH);
                cam.rotate(-rotation.m_x, delta_pitch);
            }

            if (input.hasModifier(slk::EInputModifier::ALT)) {
                g_app_state.orbitting = was_orbitting;

                if (!g_app_state.orbitting) {
                    slk::Plane const plane = {.m_normal = slk::Vector3f::UNITY, .m_dist = 0.f};

                    if (auto const res = slk::intersect(ray, plane)) {
                        g_app_state.orbitting = true;
                        g_app_state.orbit_pivot = *res;
                    }
                }
            }

            if (g_app_state.orbitting) {
                slk::Vector3f const offset = g_app_state.orbit_pivot - cam.m_pos;
                slk::Vector3f const local = {offset.dot(prev_cam_orient.xAxis()), offset.dot(prev_cam_orient.yAxis()),
                                             offset.dot(prev_cam_orient.zAxis())};

                cam.m_pos = g_app_state.orbit_pivot - (cam.right() * local.m_x + cam.up() * local.m_y + cam.forward() * local.m_z);
                translate = {};
                panning = {};

                DebugDrawEncoder dde;
                dde.begin(0);
                dde.setColor(slk::ColorU32::RED.m_abgr);
                dde.draw(bx::Sphere{.center = bx::Vec3{g_app_state.orbit_pivot.m_x, g_app_state.orbit_pivot.m_y, g_app_state.orbit_pivot.m_z},
                                    .radius = .1f});
                dde.end();
            }
        }

        cam.translate(translate);
        cam.pan(panning.m_x, panning.m_y);
    }
    // Mouse scroll w/o gesture
    else if (input.gestureTouchCount() == 0 && slk::nearZero(gesture_scroll.m_y) && mouse_scroll.m_y != 0.f) {
        slk::f32 const translate_z = mouse_scroll.m_y * 0.15f * translate_frame_factor;
        cam.translate({0.f, 0.f, translate_z});
    }

    // Elevation
    if (((input.gestureTouchCount() >= 2) || input.isMouseButtonDown(slk::EMouseButton::RIGHT)) &&
        input.isAnyKeyboardKeyDown(
            std::array{slk::EKeyboardVKey::CHAR_Q, slk::EKeyboardVKey::CHAR_E, slk::EKeyboardVKey::CHAR_R, slk::EKeyboardVKey::CHAR_F})) {
        slk::b8 const is_up = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_E, slk::EKeyboardVKey::CHAR_R});
        slk::b8 const is_down = input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_Q, slk::EKeyboardVKey::CHAR_F});

        slk::f32 y_offset = 0;
        y_offset -= is_down * app_state.input_settings.camera_translate_speed;
        y_offset += is_up * app_state.input_settings.camera_translate_speed;

        if (input.isAnyKeyboardKeyDown(std::array{slk::EKeyboardVKey::CHAR_R, slk::EKeyboardVKey::CHAR_F})) {
            cam.pan(0.f, y_offset);
        } else {
            cam.m_pos.m_y += y_offset;
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

    g_app_state.camera.setLookAt(CAMERA_RESET_POSITION, CAMERA_RESET_TARGET);
    g_app_state.camera.setPerspectiveProjection(0.1f, 10000.0f, 45.0_deg, sapp_widthf() / sapp_heightf());

    g_app_state.input_settings.camera_rotation_speed = CAMERA_DEFAULT_ROTATE_SPEED;
    g_app_state.input_settings.camera_translate_speed = CAMERA_DEFAULT_TRANSLATE_SPEED;

    g_app_state.last_frame_time_point = std::chrono::high_resolution_clock::now();
    g_app_state.last_frame_time_ms = 0.f;

    g_app_state.demos.initialize({emptyDemo(), curvesDemo()});
    g_app_state.demos.setCurrent(EMPTY_DEMO_ID);

    // debug draw init
    ddInit();
}

void appUpdate() {
    constexpr slk::ColorU32 THIN_COLOR{255, 161, 161, 161};
    constexpr slk::ColorU32 THICK_COLOR{255, 83, 83, 83};

    slk::InputApi* const input = slk::InputApi::instance();
    input->update();

    if (input->isKeyboardKeyDown(slk::EKeyboardVKey::ESCAPE)) {
        sapp_request_quit();
        return;
    }

    if (input->isKeyboardKeyDown(slk::EKeyboardVKey::BACKSLASH) &&
        !anyMaskFlagSet(g_app_state.demos.currentDesc().info.caps, EDemoCapsMask::CUSTOM_CAMERA_CONTROL))
        g_app_state.camera.setLookAt(CAMERA_RESET_POSITION, CAMERA_RESET_TARGET);

    auto const time_point = AppState::Timer::now();
    g_app_state.last_frame_time_ms =
        std::chrono::duration_cast<std::chrono::microseconds>(time_point - g_app_state.last_frame_time_point).count() / 1000.f;
    g_app_state.last_frame_time_point = time_point;

    if (!g_app_state.imgui_wnd_focused)
        updateCamera(g_app_state, g_app_state.camera, *input);

    if (!g_app_state.demos.update(g_app_state.last_frame_time_ms, g_app_state.camera)) {
        sapp_request_quit();
        return;
    }

    slk::Matrix4f const proj = g_app_state.camera.projectionMatrix(slk::EHandedness::DEFAULT, bgfx::getCaps()->homogeneousDepth);
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

static ImGuiKey toImGuiKey(sapp_keycode key) {
    // Contiguous ranges in both enums
    if ((key >= SAPP_KEYCODE_A) && (key <= SAPP_KEYCODE_Z))
        return static_cast<ImGuiKey>(ImGuiKey_A + (key - SAPP_KEYCODE_A));
    if ((key >= SAPP_KEYCODE_0) && (key <= SAPP_KEYCODE_9))
        return static_cast<ImGuiKey>(ImGuiKey_0 + (key - SAPP_KEYCODE_0));
    if ((key >= SAPP_KEYCODE_F1) && (key <= SAPP_KEYCODE_F24))
        return static_cast<ImGuiKey>(ImGuiKey_F1 + (key - SAPP_KEYCODE_F1));
    if ((key >= SAPP_KEYCODE_KP_0) && (key <= SAPP_KEYCODE_KP_9))
        return static_cast<ImGuiKey>(ImGuiKey_Keypad0 + (key - SAPP_KEYCODE_KP_0));

    switch (key) {
        case SAPP_KEYCODE_SPACE:
            return ImGuiKey_Space;
        case SAPP_KEYCODE_APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case SAPP_KEYCODE_COMMA:
            return ImGuiKey_Comma;
        case SAPP_KEYCODE_MINUS:
            return ImGuiKey_Minus;
        case SAPP_KEYCODE_PERIOD:
            return ImGuiKey_Period;
        case SAPP_KEYCODE_SLASH:
            return ImGuiKey_Slash;
        case SAPP_KEYCODE_SEMICOLON:
            return ImGuiKey_Semicolon;
        case SAPP_KEYCODE_EQUAL:
            return ImGuiKey_Equal;
        case SAPP_KEYCODE_LEFT_BRACKET:
            return ImGuiKey_LeftBracket;
        case SAPP_KEYCODE_BACKSLASH:
            return ImGuiKey_Backslash;
        case SAPP_KEYCODE_RIGHT_BRACKET:
            return ImGuiKey_RightBracket;
        case SAPP_KEYCODE_GRAVE_ACCENT:
            return ImGuiKey_GraveAccent;
        case SAPP_KEYCODE_WORLD_1:
            return ImGuiKey_Oem102;
        case SAPP_KEYCODE_ESCAPE:
            return ImGuiKey_Escape;
        case SAPP_KEYCODE_ENTER:
            return ImGuiKey_Enter;
        case SAPP_KEYCODE_TAB:
            return ImGuiKey_Tab;
        case SAPP_KEYCODE_BACKSPACE:
            return ImGuiKey_Backspace;
        case SAPP_KEYCODE_INSERT:
            return ImGuiKey_Insert;
        case SAPP_KEYCODE_DELETE:
            return ImGuiKey_Delete;
        case SAPP_KEYCODE_RIGHT:
            return ImGuiKey_RightArrow;
        case SAPP_KEYCODE_LEFT:
            return ImGuiKey_LeftArrow;
        case SAPP_KEYCODE_DOWN:
            return ImGuiKey_DownArrow;
        case SAPP_KEYCODE_UP:
            return ImGuiKey_UpArrow;
        case SAPP_KEYCODE_PAGE_UP:
            return ImGuiKey_PageUp;
        case SAPP_KEYCODE_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case SAPP_KEYCODE_HOME:
            return ImGuiKey_Home;
        case SAPP_KEYCODE_END:
            return ImGuiKey_End;
        case SAPP_KEYCODE_CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case SAPP_KEYCODE_SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case SAPP_KEYCODE_NUM_LOCK:
            return ImGuiKey_NumLock;
        case SAPP_KEYCODE_PRINT_SCREEN:
            return ImGuiKey_PrintScreen;
        case SAPP_KEYCODE_PAUSE:
            return ImGuiKey_Pause;
        case SAPP_KEYCODE_KP_DECIMAL:
            return ImGuiKey_KeypadDecimal;
        case SAPP_KEYCODE_KP_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case SAPP_KEYCODE_KP_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case SAPP_KEYCODE_KP_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case SAPP_KEYCODE_KP_ADD:
            return ImGuiKey_KeypadAdd;
        case SAPP_KEYCODE_KP_ENTER:
            return ImGuiKey_KeypadEnter;
        case SAPP_KEYCODE_KP_EQUAL:
            return ImGuiKey_KeypadEqual;
        case SAPP_KEYCODE_LEFT_SHIFT:
            return ImGuiKey_LeftShift;
        case SAPP_KEYCODE_LEFT_CONTROL:
            return ImGuiKey_LeftCtrl;
        case SAPP_KEYCODE_LEFT_ALT:
            return ImGuiKey_LeftAlt;
        case SAPP_KEYCODE_LEFT_SUPER:
            return ImGuiKey_LeftSuper;
        case SAPP_KEYCODE_RIGHT_SHIFT:
            return ImGuiKey_RightShift;
        case SAPP_KEYCODE_RIGHT_CONTROL:
            return ImGuiKey_RightCtrl;
        case SAPP_KEYCODE_RIGHT_ALT:
            return ImGuiKey_RightAlt;
        case SAPP_KEYCODE_RIGHT_SUPER:
            return ImGuiKey_RightSuper;
        case SAPP_KEYCODE_MENU:
            return ImGuiKey_Menu;
        default:
            return ImGuiKey_None;
    }
}

// The bgfx imgui wrapper only reads the keyboard through bgfx's entry system (USE_ENTRY), which isn't used with sokol
static void forwardKeyToImGui(sapp_event const* evt, bool down) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Shift, (evt->modifiers & SAPP_MODIFIER_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Ctrl, (evt->modifiers & SAPP_MODIFIER_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (evt->modifiers & SAPP_MODIFIER_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (evt->modifiers & SAPP_MODIFIER_SUPER) != 0);

    ImGuiKey const key = toImGuiKey(evt->key_code);
    if (key != ImGuiKey_None) {
        io.AddKeyEvent(key, down);
        io.SetKeyEventNativeData(key, evt->key_code, 0);
    }
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
            slk_event.m_type = slk::EInputEventType::MOUSE_MOVE;
            slk_event.m_postion = slk::Vector2f{evt->mouse_x, evt->mouse_y};

            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::EInputEventType::MOUSE_BUTTON_DOWN;
            slk_event.m_button = static_cast<slk::EMouseButton>(evt->mouse_button);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::EInputEventType::MOUSE_BUTTON_UP;
            slk_event.m_button = static_cast<slk::EMouseButton>(evt->mouse_button);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            slk::MouseEvent slk_event;
            slk_event.m_type = slk::EInputEventType::MOUSE_SCROLL;
            slk_event.m_scroll = {evt->scroll_x, evt->scroll_y};
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_DOWN: {
            forwardKeyToImGui(evt, true);

            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::EInputEventType::KEYBOARD_KEY_DOWN;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::EKeyboardVKey>(evt->key_code);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            forwardKeyToImGui(evt, false);

            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::EInputEventType::KEYBOARD_KEY_UP;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::EKeyboardVKey>(evt->key_code);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        // Note: Used for text input e.g. 'A' == VirtualKey::CHAR_A + InputModifier::SHIFT
        case SAPP_EVENTTYPE_CHAR: {
            ImGui::GetIO().AddInputCharacter(evt->char_code);

            break;
        }

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
    slk_event.m_type = slk::EInputEventType::GESTURE_TOUCH_COUNT_UPDATE;
    slk_event.m_point_count = count;
    slk::InputApi::instance()->forwardEvent(slk_event);
}

void appNotifyGestureScroll(int touch_count, float deltax, float deltay) {
    slk::GestureEvent slk_event;
    slk_event.m_type = slk::EInputEventType::GESTURE_SCROLL;
    slk_event.m_point_count = touch_count;
    slk_event.m_delta = {deltax, deltay};
    slk::InputApi::instance()->forwardEvent(slk_event);
}

void appNotifyGestureMagnify(int touch_count, float magnification) {
    slk::GestureEvent slk_event;
    slk_event.m_type = slk::EInputEventType::GESTURE_MAGNIFY;
    slk_event.m_point_count = touch_count;
    slk_event.m_delta = {magnification, 0.f};
    slk::InputApi::instance()->forwardEvent(slk_event);
}

// Mouse wheel scroll from the platform layer, used on macOS instead of sokol's SAPP_EVENTTYPE_MOUSE_SCROLL
void appNotifyMouseScroll(float deltax, float deltay) {
    slk::MouseEvent slk_event;
    slk_event.m_type = slk::EInputEventType::MOUSE_SCROLL;
    slk_event.m_scroll = {deltax, deltay};
    slk::InputApi::instance()->forwardEvent(slk_event);
}
