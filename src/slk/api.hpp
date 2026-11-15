#pragma once

#include <assert.h>

namespace slk {

class InputApi;

struct ApiRegistry {
    InputApi* m_input = nullptr;
};

// TApi declares a registry slot holding a TApi*
template <typename TApi>
concept RegistryApiType =  requires { TApi::API_SLOT; };

namespace details {
    ApiRegistry& apiRegistryInternal();
}

ApiRegistry const& apiRegistry();

template<RegistryApiType TApi>
TApi* tryApi() {
    return details::apiRegistryInternal().*TApi::API_SLOT;
}

template<RegistryApiType TApi>
TApi& api() {

    ApiRegistry const& reg = details::apiRegistryInternal();
    assert(reg.*TApi::API_SLOT);

    return *(reg.*TApi::API_SLOT);
}

template<RegistryApiType TApi>
TApi* registerApi(TApi* api_inst) {

    ApiRegistry& reg = details::apiRegistryInternal();
    TApi* const prev_api = reg.*TApi::API_SLOT;

    reg.*TApi::API_SLOT = api_inst;

    return prev_api;
}

template<RegistryApiType TApi>
TApi* unregisterApi(TApi* api_inst) {
    if (tryApi<TApi>() != api_inst)
    {
        assert(false);
        return nullptr;
    }

    return registerApi<TApi>(nullptr);
}

}

#define sb_declare_api(name) \
    static constexpr auto API_SLOT = &slk::ApiRegistry::m_##name

