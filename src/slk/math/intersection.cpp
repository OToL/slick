#include "intersection.hpp"
#include "plane.hpp"
#include "ray3.hpp"
#include "utils.hpp"

namespace slk {

std::optional<Vector3f> intersect(Ray3f const& ray, Plane const& plane) {

    f32 const dot_norm = ray.m_dir.dot(plane.m_normal);
    if (nearZero(dot_norm))
        return std::nullopt;

    f32 const t = -(ray.m_pos.dot(plane.m_normal) + plane.m_dist)/dot_norm;
    if (t < 0.f)
        return std::nullopt;

    return ray.m_pos + ray.m_dir*t;
}

}
