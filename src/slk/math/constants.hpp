#pragma once

#include <slk/core.hpp>

import std;

namespace slk {

inline constexpr f32 PI_F32 = std::numbers::pi_v<f32>;
inline constexpr f64 PI_F64 = std::numbers::pi_v<f64>;
inline constexpr f32 EPSILON_F32 = std::numeric_limits<slk::f32>::epsilon();
inline constexpr f32 EPSILON_F64 = std::numeric_limits<slk::f64>::epsilon();

}
