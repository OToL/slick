#pragma once

#include "vector3.hpp"

namespace slk {

struct Plane {
    Vector3f m_normal;
    f32 m_dist;
};

} // namespace slk
