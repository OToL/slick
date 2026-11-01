#pragma once

namespace slk {

class InputApi;

struct ApiHub {
    InputApi* m_input;
};

ApiHub apiHub();

}
