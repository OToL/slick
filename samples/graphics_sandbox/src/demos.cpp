#include "demos.hpp"
#include "demo_empty.hpp"
#include "demo_curves.hpp"
#include "demo_picking.hpp"

#include <raylib/raylib.h>

#include <cassert>
#include <vector>

struct DemosState {
    DemoId curr_demo_id = DemoId::EMPTY;
    std::vector<DemoInfo> demos_info;
    std::vector<DemoFn> demos_fn;
};

DemosState * g_demos_state = nullptr;

slk::b8 Demos::init(DemoId default_demo_id) {
    if (g_demos_state)
    {
        assert(false);
        return false;
    }

    g_demos_state = new DemosState{};
    g_demos_state->demos_fn = {
        DemoEmpty::getFn(),
        DemoCurves::getFn(),
        DemoPicking::getFn(),
    };
    g_demos_state->demos_info = {
        DemoEmpty::getInfo(),
        DemoCurves::getInfo(),
        DemoPicking::getInfo(),
    };
    g_demos_state->curr_demo_id = default_demo_id;

    return g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)].init();
}

slk::b8 Demos::shutdown() {
    if (!g_demos_state)
    {
        assert(false);
        return false;
    }

    g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)].shutdown();

    delete g_demos_state;
    g_demos_state = nullptr;

    return true;
}

DemoId Demos::current() {
    if (!g_demos_state)
    {
        assert(false);
        return DemoId::EMPTY;
    }

    return g_demos_state->curr_demo_id;
}

slk::b8 Demos::set_current(DemoId id) {
    if (!g_demos_state)
    {
        assert(false);
        return false;
    }

    DemoFn const& prev_demo_fn = g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)];
    if (!prev_demo_fn.shutdown())
    {
        assert(false);
        return false;
    }

    g_demos_state->curr_demo_id = id;
    DemoFn const& new_demo_fn = g_demos_state->demos_fn[std::to_underlying(id)];

    return new_demo_fn.init();
}

slk::b8 Demos::update(slk::f32 frame_delta_ms, Camera3D& cam3d) {
    if (!g_demos_state)
    {
        assert(false);
        return false;
    }

    return g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)].update(frame_delta_ms, cam3d);
}

slk::b8 Demos::draw3d(slk::f32 frame_delta_ms, Camera3D const& cam3d) {
    if (!g_demos_state)
    {
        assert(false);
        return false;
    }

    DemoFn const& demo_fn = g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)];
    if (demo_fn.draw3d)
        return demo_fn.draw3d(frame_delta_ms, cam3d);

    return true;
}

slk::b8 Demos::draw2d(slk::f32 frame_delta_ms) {
    if (!g_demos_state)
    {
        assert(false);
        return false;
    }

    DemoFn const& demo_fn = g_demos_state->demos_fn[std::to_underlying(g_demos_state->curr_demo_id)];
    if (demo_fn.draw2d)
        return demo_fn.draw2d(frame_delta_ms);

    return true;
}

std::span<DemoInfo const> Demos::getDemosInfo() {
    if (!g_demos_state)
    {
        return {};
    }

    return g_demos_state->demos_info;
}
