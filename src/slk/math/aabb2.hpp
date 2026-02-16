#pragma once

#include <slk/core.hpp>
#include "vector2.hpp"

namespace slk {

template <typename T>
struct AABB2 {
    using ParamType = AABB2Param<T>;
    using ScalarType = typename slk::Vector2<T>::ScalarType;
    using ExtentType = typename slk::Vector2<T>::ParamType;
    using ExtentParamType = typename ExtentType::ParamType;

    ExtentType min;
    ExtentType max;

    AABB2() = default;

    AABB2(ExtentType min_val, ExtentType max_val)
        : min(min_val)
        , max(max_val) { }

    [[nodiscard]] ScalarType width() const {
        return max.x - min.x;
    }

    [[nodiscard]] ScalarType height() const {
        return max.y - min.y;
    }

    [[nodiscard]] slk::Vector2<ScalarType> size() const {
        return max - min;
    }

    [[nodiscard]] slk::Vector2<ScalarType> half_size() const {
        return size() * 0.5;
    }

    [[nodiscard]] ExtentType center() const {
        return min + half_size();
    }

    AABB2& shrink(ExtentParamType v) {
        min += v;
        max -= v;

        return *this;
    }

    AABB2& displace(ExtentParamType v) {
        min += v;
        max += v;
        return *this;
    }

    [[nodiscard]] AABB2 displaced(ExtentParamType v) const {
        AABB2 aabb = *this;
        aabb.displace(v);

        return aabb;
    }

    AABB2& scale(ExtentParamType v) {
        ExtentType const new_half_diag = size() * v * 0.5;
        ExtentType const center = center();

        min = center - new_half_diag;
        min = center + new_half_diag;

        return *this;
    }

    [[nodiscard]] AABB2 centered_at_origin() const {
        ExtentType const half_val = half_size() * 0.5f;
        return {-half_val, half_val};
    }

    [[nodiscard]] bool is_inside(slk::Vector2<ScalarType> const& v) const {
        return min < v && v < max;
    }

    [[nodiscard]] bool is_inside(ParamType aabb) const {
        return min < aabb.min && max > aabb.max;
    }

    [[nodiscard]] bool overlaps(ParamType aabb) const {
        return (aabb.max.x > min.x && aabb.max.y > min.y) && (aabb.min.x < max.x && aabb.min.y < max.y);
    }
};

template <typename T>
[[nodiscard]] AABB2<T> operator*(AABB2Param<T> aabb, slk::f32 val) {
    return {aabb.min * val, aabb.max * val};
}

template <typename T>
[[nodiscard]] AABB2<T> operator*(AABB2Param<T> aabb, typename AABB2<T>::ExtentType const& v) {
    return {aabb.min * v, aabb.max * v};
}

using AABB2i = AABB2<slk::i32>;
using AABB2f = AABB2<slk::f32>;

} // namespace slk
