#include "demos.hpp"

using namespace std::string_view_literals;

static const DemoDesc g_demos[] = {
    {
        .init = [] () {return true;},
        .deinit = [] () {return true;},
        .update = [] (slk::f32, Camera3D&) {return true;},
        .draw3d = [] (slk::f32, Camera3D const&) {return true;},
        .draw2d = nullptr,
        .name = "Empty"sv,
        .description = "Empty Test Demo"sv,
        .id = DemoId::EMPTY,
        .caps = DemoCaps::NONE,
    },
};

std::span<DemoDesc const> getDemos() {
    return g_demos;
}
