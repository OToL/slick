#pragma once

#include <core/core.hpp>

#include <span>
#include <string_view>
#include <utility>

struct Camera3D;

enum class DemoId : slk::u32 { 
    EMPTY 
};

enum class DemoCaps : slk::u32 {
    NONE = 0,
    CAMERA_OVERRIDE = 1 << 0,
};

inline DemoCaps operator|(DemoCaps lval, DemoCaps rval) {
    return static_cast<DemoCaps>(std::to_underlying(lval) | std::to_underlying(rval));
}

inline DemoCaps operator&(DemoCaps lval, DemoCaps rval) {
    return static_cast<DemoCaps>(std::to_underlying(lval) & std::to_underlying(rval));
}

struct DemoDesc {
    slk::b8 (*init)();
    slk::b8 (*deinit)();
    slk::b8 (*update)(slk::f32 fram_delta_ms, Camera3D& cam3d);
    slk::b8 (*draw3d)(slk::f32 frame_delta_ms, Camera3D const& cam3d);
    slk::b8 (*draw2d)(slk::f32 frame_delta_ms);

    std::string_view name;
    std::string_view description;

    DemoId id;
    DemoCaps caps;
};

std::span<DemoDesc const> getDemos();
