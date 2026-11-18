#include "demo_empty.hpp"

import std;

namespace {
    struct DemoState {
    };

    slk::b8 init(DemoHdl& hdl) {
        hdl = std::malloc(sizeof(DemoState));

        return true;
    }

    slk::b8 shutdown(DemoHdl hdl) {
        std::free(hdl);

        return true;
    }

    slk::b8 update(DemoHdl /* hdl */, slk::f32 /* frame_delta_ms */, slk::Camera& /* cam */) {
        return true;
    }

    void draw3d(DemoHdl /* hdl */, slk::f32 /* frame_delta_ms */, slk::Camera const& /* cam */) {
    }

    void draw2d(DemoHdl /* hdl */, slk::f32 /* frame_delta_ms */) {
    }
}


DemoDesc emptyDemo()
{
    return {
        .m_id = EMPTY_DEMO_ID,
        .m_info = DemoInfo {
            .m_name = "Empty demo",
            .m_descriptionm = "Empty demo",
            .m_caps = EDemoCapBits::NONE
        },
        .m_api = {
            .m_init = &init,
            .m_shutdown = &shutdown,
            .m_update = &update,
            .m_drawImgui = nullptr,
            .m_draw3d = &draw3d,
            .m_draw2d = &draw2d
        }
    };
}
