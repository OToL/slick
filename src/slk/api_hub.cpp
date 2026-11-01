#include "api_hub.hpp"

#include <cassert>

namespace slk {

static constinit ApiHub g_ApiHub = {};

void registerInputApi(InputApi* api) {
    g_ApiHub.m_input = api;
}
void unregisterInputApi(InputApi* api) {
    assert(api == g_ApiHub.m_input);
    g_ApiHub.m_input = nullptr;
}

ApiHub apiHub() {
    return g_ApiHub;
}

}
