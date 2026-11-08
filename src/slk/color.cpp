#include "color.hpp"

import std;

static_assert(std::endian::native == std::endian::little, "ColorU32 memory layout (RGBA8) assumes little-endian");
