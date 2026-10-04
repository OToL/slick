#include "app.hpp"

#define SOKOL_IMPL
#include <sokol/sokol_app.h>

#import <Metal/Metal.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"

static CAMetalLayer* g_metal_layer = nullptr;

void * appGetWindowHdl() {

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

    appInitialize();
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
        .width = 1200,
        .height = 720,
        .window_title = "Asteroids",
    };

    sapp_metal_swapchain();

    sapp_run(&desc);

    return 0;
}

#pragma clang diagnostic pop
