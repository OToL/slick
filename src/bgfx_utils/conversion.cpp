#include "conversion.h"

namespace bgfx_utils {

bx::Vec3 toBx(slk::Vector3f slk_vec)
{
    return {slk_vec.m_x, slk_vec.m_y, slk_vec.m_z};
}

slk::Vector3f toSlk(bx::Vec3 bx_vec)
{
    return {bx_vec.x, bx_vec.y, bx_vec.z};
}

}

