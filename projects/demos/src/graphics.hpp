#pragma once

#include <slk/math/fwd.hpp>

#include <bgfx/bgfx.h>

namespace slk {
struct ColorU32;
}

//
// Grid System
//

// TODO: Allow larger/infinite grid (> 1000) by ...
//  1. Centering the grid at camera position (dynamic repositioning) - keeps uv values small
//  2. Using double precision (not available in GLSL 330)
//  3. Usiing a different grid calculation that doesn't rely on large absolute positions

struct GridHdlImpl;
using GridHdl = GridHdlImpl*;

GridHdl createGrid();
void destroyGrid(GridHdl grid);
void renderGrid(GridHdl grid, slk::ColorU32 const& thin_color, slk::ColorU32 const& thick_color, bool debug_visualize);

bgfx::ShaderHandle loadShader(const char* file_name);
