#pragma once

#include <slk/core.hpp>
#include "vector2.hpp"

namespace slk {

template <typename T>
struct [[nodiscard]] AABB2 {
    using ParamType = AABB2Param<T>;
    using ScalarType = typename Vector2<T>::ScalarType;
    using ExtentType = Vector2<T>;
    using ExtentParamType = typename ExtentType::ParamType;

    ExtentType m_min;
    ExtentType m_max;

    constexpr AABB2() = default;

    constexpr AABB2(ExtentParamType min_val, ExtentParamType max_val)
        : m_min(min_val)
        , m_max(max_val) { }

    constexpr ScalarType width() const {
        return m_max.m_x - m_min.m_x;
    }

    constexpr ScalarType height() const {
        return m_max.m_y - m_min.m_y;
    }

    constexpr Vector2<ScalarType> size() const {
        return m_max - m_min;
    }

    constexpr Vector2<ScalarType> halfSize() const {
        return size() * ScalarType(0.5);
    }

    constexpr ExtentType center() const {
        return m_min + halfSize();
    }

    constexpr AABB2& shrink(ExtentParamType v) {
        m_min += v;
        m_max -= v;

        return *this;
    }

    constexpr AABB2& displace(ExtentParamType v) {
        m_min += v;
        m_max += v;
        return *this;
    }

    constexpr AABB2 displaced(ExtentParamType v) const {
        return {
            m_min + v,
            m_max + v
        };
    }

    // scales the box around its center (component-wise)
    constexpr AABB2& scaleHalf(ExtentParamType v) {
        ExtentType const new_half_size = halfSize() * v;
        ExtentType const box_center = center();

        m_min = box_center - new_half_size;
        m_max = box_center + new_half_size;

        return *this;
    }

    constexpr AABB2 centeredAtOrigin() const {
        ExtentType const half_size = halfSize();
        return {-half_size, half_size};
    }

    constexpr bool contains(slk::Vector2<ScalarType> const& v) const {
        return m_min.allLess(v) && v.allLess(m_max);
    }

    constexpr bool contains(ParamType aabb) const {
        return m_min.allLess(aabb.m_min) && m_max.allGreater(aabb.m_max);
    }

    constexpr bool overlaps(ParamType aabb) const {
        return (aabb.m_max.m_x > m_min.m_x && aabb.m_max.m_y > m_min.m_y) && (aabb.m_min.m_x < m_max.m_x && aabb.m_min.m_y < m_max.m_y);
    }
};

template <typename T>
constexpr AABB2<T> operator*(AABB2Param<T> aabb, typename AABB2<T>::ScalarType val) {
    return {aabb.m_min * val, aabb.m_max * val};
}

template <typename T>
constexpr AABB2<T> operator*(AABB2Param<T> aabb, typename AABB2<T>::ExtentType const& v) {
    return {aabb.m_min * v, aabb.m_max * v};
}

using AABB2i = AABB2<slk::i32>;
using AABB2f = AABB2<slk::f32>;

} // namespace slk
