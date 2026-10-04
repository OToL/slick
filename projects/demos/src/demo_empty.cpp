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
        .id = EMPTY_DEMO_ID,
        .info = DemoInfo {
            .name = "Empty demo",
            .description = "Empty demo",
            .caps = DemoCapsMask::NONE
        },
        .api = {
            .init = &init,
            .shutdown = &shutdown,
            .update = &update,
            .drawImgui = nullptr,
            .draw3d = &draw3d,
            .draw2d = &draw2d
        }
    };
}
