#pragma once

#include <core/core.hpp>

#include <span>
#include <string_view>
#include <utility>

struct Camera3D;

enum class DemoId : slk::u32 { 
    EMPTY,
    CURVES
};

enum class DemoCaps : slk::u32 {
    NONE = 0,
    DEFAULT_CAMERA_CONTROL = 1 << 0,
};

SB_DECLARE_ENUM_MASK(DemoCaps);

struct DemoInfo {
    std::string_view name;
    std::string_view description;
    DemoCaps caps;
};

struct DemoFn {
    slk::b8 (*init)();
    slk::b8 (*shutdown)();
    slk::b8 (*update)(slk::f32 frame_delta_ms, Camera3D& cam3d);
    slk::b8 (*draw3d)(slk::f32 frame_delta_ms, Camera3D const& cam3d);
    slk::b8 (*draw2d)(slk::f32 frame_delta_ms);
};

struct Demos {
    static slk::b8 init(DemoId default_demo_id = DemoId::EMPTY);
    static slk::b8 shutdown();

    static DemoId  current();
    static slk::b8 set_current(DemoId id);

    static slk::b8 update(slk::f32 frame_delta_ms, Camera3D& cam3d);
    static slk::b8 draw3d(slk::f32 frame_delta_ms, Camera3D const& cam3d);
    static slk::b8 draw2d(slk::f32 frame_delta_ms);

    static std::span<DemoInfo const> getDemosInfo();
};

