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
        app_notify_gesture_touch_count((int)all.count);
        return event;
    }];

    // [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskSwipe
    //                                       handler:^NSEvent*(NSEvent* event) {
    //     NSSet<NSTouch*>* all = [event touchesMatchingPhase:NSTouchPhaseTouching inView:nil];
    //     app_set_touch_point_count((int)all.count);
    //     return event;
    // }];

    // NSEventMaskGesture API_AVAILABLE(macos(10.5))          = 1ULL << NSEventTypeGesture,
    // NSEventMaskMagnify API_AVAILABLE(macos(10.5))          = 1ULL << NSEventTypeMagnify,
    // NSEventMaskSwipe API_AVAILABLE(macos(10.5))            = 1ULL << NSEventTypeSwipe,
    // NSEventMaskRotate API_AVAILABLE(macos(10.5))           = 1ULL << NSEventTypeRotate,

    [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskScrollWheel
                                          handler:^NSEvent*(NSEvent* event) {

        // Once fingers leave the trackpad, macOS keeps synthesizing scroll
        // events with decaying deltas to produce the "coasting" inertia
        // effect. Those carry a non-zero momentumPhase, unlike events driven
        // directly by finger movement (momentumPhase == NSEventPhaseNone).
        // Drop them so drags stop the instant the user lifts their fingers.
        if (event.momentumPhase != NSEventPhaseNone) {
            return event;
        }

        float deltax = static_cast<float>(event.scrollingDeltaX);
        float deltay = static_cast<float>(event.scrollingDeltaY);

        // STANDARD MOUSE WHEEL (Logitech, Razer, etc.)
        // Apple scales a standard wheel click down to roughly 0.1 to 1.0.
        // Multiply by a fixed scalar to restore standard hardware wheel "ticks".
        // Cross-platform frameworks (like GLFW) use a multiplier of ~0.1 to 1.0 
        // depending on how they define 1 "tick", but if you want it to feel like Windows:
        if (!event.hasPreciseScrollingDeltas) {
            float const WHEEL_TICK_MULTIPLIER = 30.f; 
            deltax *= WHEEL_TICK_MULTIPLIER;
            deltay *= WHEEL_TICK_MULTIPLIER;
        }

        app_notify_gesture_scroll(2, deltax, deltay);
        return event;
    }];

    [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMagnify
                                          handler:^NSEvent*(NSEvent* event) {
        NSSet<NSTouch*>* all = [event touchesMatchingPhase:NSTouchPhaseTouching inView:nil];
        app_notify_gesture_touch_count((int)all.count);
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
