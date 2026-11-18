#pragma once

#include <bx/math.h>
#include <slk/math/vector3.hpp>

namespace bgfx_utils {

bx::Vec3 toBx(slk::Vector3f slk_vec);

slk::Vector3f toSlk(bx::Vec3 bx_vec);


}


