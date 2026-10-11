#include "app.hpp"

#define SOKOL_IMPL
#include <sokol/sokol_app.h>

#include <bgfx/bgfx.h>

#import <Metal/Metal.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"

static CAMetalLayer* g_metal_layer = nullptr;

// Average touch position from the previous gesture event, used to compute movement
static NSPoint g_prev_centroid = {};
static NSUInteger g_prev_count = 0;
// Sum of centroid movement not yet sent; the trackpad's ±1 sensor step flicker cancels out in it
static constexpr float GESTURE_JITTER_PT = 0.15f;
static NSPoint g_pending_delta = {};

void * appGetWindowHdl() {

    return g_metal_layer;
}

void appTriggerGpuCapture() {
    MTLCaptureManager* mgr = [MTLCaptureManager sharedCaptureManager];

    if (![mgr supportsDestination:MTLCaptureDestinationGPUTraceDocument]) {
        NSLog(@"GPU capture: MTLCaptureDestinationGPUTraceDocument not supported. Did you set MTL_CAPTURE_ENABLED=1 ?");
        return;
    }

    MTLCaptureDescriptor* desc = [MTLCaptureDescriptor new];
    desc.captureObject = g_metal_layer.device;
    desc.destination = MTLCaptureDestinationGPUTraceDocument;

    NSString* path = [NSString stringWithFormat:@"/tmp/graphics_sandbox_%d.gputrace", (int)getpid()];
    desc.outputURL = [NSURL fileURLWithPath:path];

    NSError* error = nil;
    if (![mgr startCaptureWithDescriptor:desc error:&error]) {
        NSLog(@"GPU capture: failed to start: %@", error);
        return;
    }

    NSLog(@"GPU capture: started, writing to %@", path);
}

void appStopGpuCapture() {
    MTLCaptureManager* mgr = [MTLCaptureManager sharedCaptureManager];
    if (mgr.isCapturing) {
        [mgr stopCapture];
        NSLog(@"GPU capture: stopped");
    }
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

    appInitialize();

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
    NSUInteger const count = all.count;
    appNotifyGestureTouchCount((int)count);

    if (count >= 3) {
        NSPoint centroid = {};
        NSSize device_size = {};
        for (NSTouch* t in all) {
            centroid.x += t.normalizedPosition.x;
            centroid.y += t.normalizedPosition.y;
            device_size = t.deviceSize; // trackpad size in points
        }
        centroid.x /= count;
        centroid.y /= count;

        // Only send a delta if the finger count hasn't changed, otherwise adding or lifting a finger causes a jump
        if (count == g_prev_count) {
            g_pending_delta.x += (centroid.x - g_prev_centroid.x) * device_size.width;
            g_pending_delta.y += (g_prev_centroid.y - centroid.y) * device_size.height; // trackpad Y points up

            if ((g_pending_delta.x * g_pending_delta.x + g_pending_delta.y * g_pending_delta.y) > (GESTURE_JITTER_PT * GESTURE_JITTER_PT)) {
                appNotifyGestureScroll((int)count, (float)g_pending_delta.x, (float)g_pending_delta.y);
                g_pending_delta = {};
            }
        }
        else {
            g_pending_delta = {};
        }
        g_prev_centroid = centroid;
    }
    g_prev_count = count;
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


        bool const is_gesture = (event.phase != NSEventPhaseNone) || (event.momentumPhase != NSEventPhaseNone);
        if (is_gesture)
            appNotifyGestureScroll(2, deltax, deltay);

        return event;
    }];

    [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMagnify
                                          handler:^NSEvent*(NSEvent* event) {
        // harcode 2 because magnify do not pass any touch
        appNotifyGestureMagnify(2, (float)event.magnification);
        return event;
    }];
}

void darwin_cleanup_wrapper() {
    appShutdown();

    NSWindow* wnd = (__bridge NSWindow*) sapp_macos_get_window();
    NSView* view = wnd.contentView;

    view.layer = nil;     // remove our CAMetalLayer
    view.wantsLayer = NO;

    g_metal_layer = nullptr;
}

int main() {
    sapp_desc desc = {
        .init_cb = darwin_init_wrapper,
        .frame_cb = &appUpdate,
        .cleanup_cb = &darwin_cleanup_wrapper,
        .event_cb = &appNotifyEvent,
        .width = 1280,
        .height = 720,
        .window_title = "Graphics Sandbox",
    };

    sapp_metal_swapchain();

    sapp_run(&desc);

    return 0;
}

#pragma clang diagnostic pop

