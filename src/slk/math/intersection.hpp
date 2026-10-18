#pragma once

#include "vector3.hpp"
#include "fwd.hpp"

import std;

namespace slk {

std::optional<Vector3f> intersect(Ray3f const& ray, Plane const& plane);

}
