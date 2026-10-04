#include "demos.hpp"

#include <cassert>

import std;

slk::b8 Demos::initialize(std::initializer_list<DemoDesc>const& descs) 
{
    // already init
    if (!m_demos.empty()) {
        return false;
    }

    m_curr_idx = INVALID_DEMO_IDX;
    m_demos = descs;
    return true;
}

slk::b8 Demos::shutdown() {
    if (m_demos.empty()) {
        return false;
    }

    if (m_curr_hdl)
    {
        m_demos[m_curr_idx].api.shutdown(m_curr_hdl);
        m_curr_hdl = nullptr;
    }

    m_demos.clear();

    return true;
}

DemoDesc Demos::currentDesc() const {
    if (!m_curr_hdl)
        return {};

    return m_demos[m_curr_idx];
}

DemoId  Demos::currentId() const {
    if (!m_curr_hdl)
        return {};

    return m_demos[m_curr_idx].id;
}

slk::b8 Demos::setCurrent(DemoId id)
{
    if (m_demos.empty())
    {
        return false;
    }

    auto const demo_it = std::ranges::find_if(m_demos, [id] (DemoDesc const& desc) {
        return strcmp(desc.id, id) == 0;
    });

    if (demo_it == m_demos.end()) {
        return false;
    }

    if (m_curr_hdl) {
        slk::b8 const res = m_demos[m_curr_idx].api.shutdown(m_curr_hdl);
        assert(res);

        m_curr_hdl = nullptr;
        m_curr_idx = INVALID_DEMO_IDX;
    }

    slk::b8 const res = demo_it->api.init(m_curr_hdl);
    if (!res)
    {
        assert(!m_curr_hdl);
        return false;
    }

    m_curr_idx = std::distance(m_demos.begin(), demo_it);

    return true;
}

slk::b8 Demos::update(slk::f32 frame_delta_ms, slk::Camera& cam) {
    if (!m_curr_hdl)
        return false;

    return m_demos[m_curr_idx].api.update(m_curr_hdl, frame_delta_ms, cam);
}

slk::b8 Demos::draw3d(slk::f32 frame_delta_ms, slk::Camera const& cam)
{
    if (!m_curr_hdl)
        return false;

    m_demos[m_curr_idx].api.draw3d(m_curr_hdl, frame_delta_ms, cam);
    return true;
}

slk::b8 Demos::draw2d(slk::f32 frame_delta_ms) {
    if (!m_curr_hdl)
        return false;

    m_demos[m_curr_idx].api.draw2d(m_curr_hdl, frame_delta_ms);
    return true;
}

slk::b8 Demos::drawImgui(slk::f32 frame_delta_ms)
{
    if (!m_curr_hdl)
        return false;

    m_demos[m_curr_idx].api.drawImgui(m_curr_hdl, frame_delta_ms);
    return true;
}


