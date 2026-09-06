#pragma once

#include <slk/core.hpp>
#include "vector2.hpp"

namespace slk {

template <typename T>
struct AABB2 {
    using ParamType = AABB2Param<T>;
    using ScalarType = typename Vector2<T>::ScalarType;
    using ExtentType = typename Vector2<T>::ParamType;
    using ExtentParamType = typename ExtentType::ParamType;

    ExtentType m_min;
    ExtentType m_max;

    constexpr AABB2() = default;

    constexpr AABB2(ExtentType min_val, ExtentType max_val)
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

    constexpr Vector2<ScalarType> half_size() const {
        return size() * 0.5;
    }

    constexpr ExtentType center() const {
        return m_min + half_size();
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
        AABB2 aabb = *this;
        aabb.displace(v);

        return aabb;
    }

    constexpr AABB2& scale(ExtentParamType v) {
        ExtentType const new_half_diag = size() * v * 0.5;
        ExtentType const center = center();

        m_min = center - new_half_diag;
        m_min = center + new_half_diag;

        return *this;
    }

    constexpr AABB2 centeredAtOrigin() const {
        ExtentType const half_val = half_size() * 0.5f;
        return {-half_val, half_val};
    }

    constexpr bool isInside(slk::Vector2<ScalarType> const& v) const {
        return m_min < v && v < m_max;
    }

    constexpr bool isInside(ParamType aabb) const {
        return m_min < aabb.m_min && m_max > aabb.m_max;
    }

    constexpr bool overlaps(ParamType aabb) const {
        return (aabb.m_max.m_x > m_min.m_x && aabb.m_max.m_y > m_min.m_y) && (aabb.m_min.m_x < m_max.m_x && aabb.m_min.m_y < m_max.m_y);
    }
};

template <typename T>
constexpr AABB2<T> operator*(AABB2Param<T> aabb, slk::f32 val) {
    return {aabb.m_min * val, aabb.m_max * val};
}

template <typename T>
constexpr AABB2<T> operator*(AABB2Param<T> aabb, typename AABB2<T>::ExtentType const& v) {
    return {aabb.m_min * v, aabb.m_max * v};
}

using AABB2i = AABB2<slk::i32>;
using AABB2f = AABB2<slk::f32>;

} // namespace slk
