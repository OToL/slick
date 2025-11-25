#include "demo_empty.hpp"

using namespace std::string_view_literals;

DemoInfo DemoEmpty::getInfo() {
    return {
        .name = "Empty"sv,
        .description = "Empty Test Demo"sv,
        .caps = DemoCaps::DEFAULT_CAMERA_CONTROL,
    };
}
DemoFn DemoEmpty::getFn() {
    return {
        .init = []() { return true; },
        .shutdown = []() { return true; },
        .update = [](slk::f32, Camera3D&) { return true; },
        .draw3d = [](slk::f32, Camera3D const&) { return true; },
        .draw2d = nullptr,
    };
}
