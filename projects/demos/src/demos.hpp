#pragma once

#include <slk/core.hpp>

#include <string_view>

import std;

namespace slk {
    struct Camera;
}

// TODO: Change to hash
using DemoId = char const*;
using DemoHdl = void*;

enum class DemoCapsMask {
    NONE = 0,
    CUSTOM_CAMERA_CONTROL = 1 << 0,
    DEBUG_IMGUI = 1 << 1,
};

sb_declare_enum_mask_operators(DemoCapsMask)

struct DemoInfo {
    std::string_view name;
    std::string_view description;
    DemoCapsMask caps;
};

struct DemoApi {
    slk::b8 (*init)(DemoHdl& hdl);
    slk::b8 (*shutdown)(DemoHdl hdl);
    slk::b8 (*update)(DemoHdl hdl, slk::f32 frame_delta_ms, slk::Camera& cam);
    void (*drawImgui)(DemoHdl hdl, slk::f32 frame_delta_ms);
    void (*draw3d)(DemoHdl hdl, slk::f32 frame_delta_ms, slk::Camera const& cam);
    void (*draw2d)(DemoHdl hdl, slk::f32 frame_delta_ms);
};

struct DemoDesc {
    DemoId id;
    DemoInfo info;
    DemoApi api;
};

class Demos {

public:
    slk::b8 initialize(std::initializer_list<DemoDesc>const& descs);
    slk::b8 shutdown();

    std::span<DemoDesc const> demos() const {return m_demos;};
    DemoDesc currentDesc() const;
    DemoId  currentId() const;
    slk::b8 setCurrent(DemoId id);

    slk::b8 update(slk::f32 frame_delta_ms, slk::Camera& cam);
    slk::b8 draw3d(slk::f32 frame_delta_ms, slk::Camera const& cam);
    slk::b8 draw2d(slk::f32 frame_delta_ms);
    slk::b8 drawImgui(slk::f32 frame_delta_ms);

private:

    static constexpr slk::i32 INVALID_DEMO_IDX = -1;

    DemoHdl m_curr_hdl = nullptr;
    slk::i32 m_curr_idx = INVALID_DEMO_IDX;
    std::vector<DemoDesc> m_demos;
};

