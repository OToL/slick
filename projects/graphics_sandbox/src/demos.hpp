#pragma once

#include <slk/core.hpp>

#include <span>
#include <string_view>

struct Camera3D;

struct DemoIdEnum {
    enum Type : slk::u32 { 
        EMPTY,
        CURVES,
        PICKING
    };
};
using EDemoId = DemoIdEnum::Type;

struct DemoCapsFlag {
    enum Type : slk::u32 {
        NONE = 0,
        DEFAULT_CAMERA_CONTROL = 1 << 0,
    };
};
using FDemoCaps = DemoCapsFlag::Type;

struct DemoInfo {
    std::string_view name;
    std::string_view description;
    FDemoCaps caps;
};

struct DemoFn {
    slk::b8 (*init)();
    slk::b8 (*shutdown)();
    slk::b8 (*update)(slk::f32 frame_delta_ms, Camera3D& cam3d);
    slk::b8 (*draw3d)(slk::f32 frame_delta_ms, Camera3D const& cam3d);
    slk::b8 (*draw2d)(slk::f32 frame_delta_ms);
};

struct Demos {
    static slk::b8 init(EDemoId default_demo_id = EDemoId::EMPTY);
    static slk::b8 shutdown();

    static EDemoId  current();
    static slk::b8 set_current(EDemoId id);

    static slk::b8 update(slk::f32 frame_delta_ms, Camera3D& cam3d);
    static slk::b8 draw3d(slk::f32 frame_delta_ms, Camera3D const& cam3d);
    static slk::b8 draw2d(slk::f32 frame_delta_ms);

    static std::span<DemoInfo const> getDemosInfo();
};

