#pragma once

#include <slk/core.hpp>
#include <slk/hash.hpp>

#include <string_view>

import std;

namespace slk {
    struct Camera;
}

using DemoId = slk::Hash64;
using DemoHdl = void*;

enum class EDemoCapBits {
    NONE = 0,
    CUSTOM_CAMERA_CONTROL = 1 << 0,
    DEBUG_IMGUI = 1 << 1,
};

sb_declare_enum_mask_operators(EDemoCapBits)

struct DemoInfo {
    std::string_view m_name;
    std::string_view m_descriptionm;
    EDemoCapBits m_caps;
};

struct DemoApi {
    slk::b8 (*m_init)(DemoHdl& hdl);
    slk::b8 (*m_shutdown)(DemoHdl hdl);
    slk::b8 (*m_update)(DemoHdl hdl, slk::f32 frame_delta_ms, slk::Camera& cam);
    void (*m_drawImgui)(DemoHdl hdl, slk::f32 frame_delta_ms);
    void (*m_draw3d)(DemoHdl hdl, slk::f32 frame_delta_ms, slk::Camera const& cam);
    void (*m_draw2d)(DemoHdl hdl, slk::f32 frame_delta_ms);
};

struct DemoDesc {
    DemoId m_id;
    DemoInfo m_info;
    DemoApi m_api;
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

