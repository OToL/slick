#include "api_registry.hpp"

#include <cassert>


namespace slk {

static constinit ApiRegistry g_registry = {};

namespace details {

ApiRegistry& apiRegistryInternal() {
    return g_registry;
};

}

void registerInputApi(InputApi* api) {
    g_registry.m_input = api;
}
void unregisterInputApi(InputApi* api) {
    assert(api == g_registry.m_input);
    g_registry.m_input = nullptr;
}

ApiRegistry const& apiRegistry() {
    return g_registry;
}

}
