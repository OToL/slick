#include "app.hpp"

#define SOKOL_IMPL
#include <sokol/sokol_app.h>

#include <bgfx/bgfx.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"

static CAMetalLayer* g_metal_layer = nullptr;

void * app_get_window_hdl() {

    return g_metal_layer;
}

void darwin_init_wrapper()
{
    NSWindow* wnd = (__bridge NSWindow*) sapp_macos_get_window();
    NSView* view = [wnd contentView];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    g_metal_layer = [CAMetalLayer layer];
    g_metal_layer.device = device;
    g_metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    view.wantsLayer = YES;
    view.layer = g_metal_layer;

    app_init();

    // Enable resting touches on the view so the system generates gesture events
    // for fingers that are placed but not yet moving.
    view.allowedTouchTypes = NSTouchTypeMaskDirect | NSTouchTypeMaskIndirect;
    view.wantsRestingTouches = YES;

    // Intercept gesture events at the application level, before Sokol sees them.
    // NSEventMaskGesture covers raw trackpad touch events and supports
    // touchesMatchingPhase:inView: (unlike scroll wheel events).
    [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskGesture
                                          handler:^NSEvent*(NSEvent* event) {
        NSSet<NSTouch*>* all = [event touchesMatchingPhase:NSTouchPhaseTouching inView:nil];
        app_set_touch_point_count((int)all.count);
        return event;
    }];
}

void darwin_cleanup_wrapper() {
    app_cleanup();

    NSWindow* wnd = (__bridge NSWindow*) sapp_macos_get_window();
    NSView* view = wnd.contentView;

    view.layer = nil;     // remove our CAMetalLayer
    view.wantsLayer = NO;

    g_metal_layer = nullptr;
}

int main() {
    sapp_desc desc = {
        .init_cb = darwin_init_wrapper,
        .frame_cb = app_frame,
        .cleanup_cb = darwin_cleanup_wrapper,
        .event_cb = app_event,
        .width = 1280,
        .height = 720,
        .window_title = "Graphics Sandbox",
    };

    sapp_metal_swapchain();

    sapp_run(&desc);

    return 0;
}

#pragma clang diagnostic pop
