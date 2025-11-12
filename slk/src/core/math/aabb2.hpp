#pragma once

#include <core/core.hpp>
#include <core/math/vector2.hpp>

namespace slk {

template <typename T>
struct AABB2 {
    using ExtentType = slk::Vector2<T>;
    using ScalarType = typename slk::Vector2<T>::ScalarType;

    ExtentType min;
    ExtentType max;

    AABB2() = default;
    AABB2(ExtentType min_val, ExtentType max_val)
        : min(min_val)
        , max(max_val) { }

    ScalarType width() const {
        return max.x - min.x;
    }

    ScalarType height() const {
        return max.y - min.y;
    }

    slk::Vector2<ScalarType> size() const {
        return max - min;
    }

    slk::Vector2<ScalarType> halfSize() const {
        return size() * 0.5;
    }

    ExtentType center() const {
        return min + halfSize();
    }

    AABB2 & shrink(ExtentType const & v) {
        min += v;
        max -= v;

        return *this;
    }

    AABB2 & displace(ExtentType const & v) {
        min += v;
        max += v;
        return *this;
    }

    AABB2 displaced(ExtentType const & v) const {
        AABB2 aabb = *this;
        aabb.displace(v);

        return aabb;
    }

    AABB2 & scale(ExtentType const & v) {
        ExtentType const new_half_diag = size() * v * 0.5;
        ExtentType const center = center();

        min = center - new_half_diag;
        min = center + new_half_diag;

        return *this;
    }

    AABB2 centeredAtOrigin() const {
        ExtentType const half_val = halfSize() * 0.5f;
        return {-half_val, half_val};
    }

    bool isInside(slk::Vector2<ScalarType> const & v) const {
        return min < v && v < max;
    }

    bool isInside(AABB2 const & aabb) const {
        return min < aabb.min && max > aabb.max;
    }

    bool overlaps(AABB2 const & aabb) const {
        return (aabb.max.x > min.x && aabb.max.y > min.y) && (aabb.min.x < max.x && aabb.min.y < max.y);
    }
};

template <typename T>
AABB2<T> operator*(AABB2<T> const & aabb, slk::f32 val) {
    return {aabb.min * val, aabb.max * val};
}

template <typename T>
AABB2<T> operator*(AABB2<T> const & aabb, typename AABB2<T>::ExtentType const & v) {
    return {aabb.min * v, aabb.max * v};
}

using AABB2i = AABB2<slk::i32>;
using AABB2f = AABB2<slk::f32>;

} // namespace slk
