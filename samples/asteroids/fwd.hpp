#pragma once

#include <cstdlib>
#include <core/math/fwd.hpp>

namespace math {

    template <typename T>
    struct AABB2;

    struct Color;

    using AABB2i = AABB2<int>;
    using AABB2f = AABB2<float>;

}

