#pragma once

#include <raylib/raylib.h>

namespace graphics {

struct GridHdl;
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
