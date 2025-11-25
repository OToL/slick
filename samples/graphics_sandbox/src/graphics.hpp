#pragma once

#include <raylib/raylib.h>

namespace graphics {

struct GridHdl;

// TODO: Allow larger/infinite grid (> 1000) by ...
//  1. Centering the grid at camera position (dynamic repositioning) - keeps uv values small
//  2. Using double precision (not available in GLSL 330)
//  3. Usiing a different grid calculation that doesn't rely on large absolute positions

GridHdl* create_grid();
void destroy_grid(GridHdl* grid);
void render_grid(GridHdl* grid,
        float cell_size,
        float grid_size,
        Matrix const& view_proj,
        Vector3 camera_world_pos,
        Color const& thin_color,
        Color const& thick_color);
}
