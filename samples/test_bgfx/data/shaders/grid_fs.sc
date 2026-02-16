// vector between the camera and the corresponing grid point
// fragment position in grid space [-grid_size; grid_size]
$input v_view_dir, v_grid_position

#include <bgfx.sh>

uniform vec4 u_grid_scalar_params;
uniform vec4 u_thin_color;
uniform vec4 u_thick_color;

/////////////////////////////////////////////////
// Utilities
/////////////////////////////////////////////////

// glsl does not have base 10 log
float log10(float x) {
    return log(x) / log(10.0);
}

vec2 saturate2(vec2 v2) {
    return vec2(saturate(v2.x), saturate(v2.y));
}

// jupyter notebook: https://colab.research.google.com/drive/1vIkHz_U8FYJWmO9ecn-poRXt6RFAWUxn
void main() {
    
    float grid_size = u_grid_scalar_params.x;
    float cell_size = u_grid_scalar_params.y;
    float distance_fade_mode = u_grid_scalar_params.z;
    float dbg_viz = u_grid_scalar_params.w;

    // position in grid space i.e. in (-grid_extent, +grid_extent) of the current pixel
    vec2 uv = v_grid_position;

    // how much grid coords are changing, in both x & y, in 1 pixel (u & v direction)
    vec2 dudv = vec2(length(vec2(dFdx(uv.x), dFdy(uv.x))),
                    length(vec2(dFdx(uv.y), dFdy(uv.y))));

    // minimum number of pixels between 2 (cell) lines from which we change the LOD
    float min_pixels_between_cells_line = 1.f;

    // compute the current pixel LOD in world space
    //   1. length(dudv) : how much grid space change in one pixel
    //   2. prev * min_pixels_between_cells_line: how much grid distance represents the minimum space, in screen space, before changing LOD
    //   3. prev / cell_size: how many cells the minimum space, in screen space, before changing LOD
    //   4. log10(prev): get the LOD level, it works because we change every 10 lines i.e. each LOD is 10x: LOD0 (10^0), LOD1 (10), LOD2 (10x10), etc.
    //   5. prev + 1: since the switch between LOD 0 to 1 is when we are close to the min distance (i.e. dudv*min_pixels_between_cells_line == cell_size),
    //      log10(1) represents the beginning of LOD1
    //   6. max(0, prev): we don't want a negative LOD
    float lod_level = max(0.f, log10((length(dudv) * min_pixels_between_cells_line) / cell_size) + 1.f);

    // get the fractional part of the previously computed floating point LOD e.g. fract(2.3) => 0.3
    // means how far we are from the current lod aka fade
    float lod_fade = fract(lod_level);

    // computes the 'cell size' for each grid stage
    // I m a bit confused we have 2 different colors for non-thin lines ... but this is how it is
    // - thin_cs: matches the current LOD cell size
    // - mid_cs : 10x thin_cs because we have different color every 10 thin lines/cells
    // - thick_cs: 100x thin_cs ... very large, must be very thick
    float thin_cs = cell_size * pow(10, floor(lod_level));
    float mid_cs = thin_cs * 10.f;
    float thick_cs = mid_cs * 10.f;

    dudv *= 2.f;

    // compute how much the pixel is close to each grid stage line within 1 pixel
    // i.e. if within 1 pixel must be colored according to the line and more, it is transparent
    // use a inverted V pattern peaking 1 pixel from grid stage/line boundaries
    // - mod(uv, thin_cs): how far we are in grid space from the previous thin line
    // - prev/dudv: how many pixels this distance represents on screen 
    // - saturate(prev): we are only interested in the 0-1 distance i.e. 1 pixel away from the grid stage boundaries
    // - 1- abs(prev*2 -1): inverted V transformation
    // because we use the distance from grid stage boundary/line, the line starts from the grid stage boundaries instead of being around it
    vec2 thin_cs_v_a = 1.f - abs(saturate2(mod(uv, thin_cs)/dudv) * 2.f - 1.f);
    // Pick the most covered value
    float thin_cs_a = max(thin_cs_v_a.x, thin_cs_v_a.y);
    // same for mid
    vec2 mid_cs_v_a = 1.f - abs(saturate2(mod(uv, mid_cs)/dudv) * 2.f - 1.f);
    float mid_cs_a = max(mid_cs_v_a.x, mid_cs_v_a.y);
    // same for thick
    vec2 thick_cs_v_a = 1.f - abs(saturate2(mod(uv, thick_cs)/dudv) * 2.f - 1.f);
    float thick_cs_a = max(thick_cs_v_a.x, thick_cs_v_a.y);

    // Blend between falloff colors to handle LOD transition [4]
    vec4 color = thick_cs_a > 0.f ? u_thick_color : mid_cs_a > 0.f ? mix(u_thick_color, u_thin_color, lod_fade) : u_thin_color;

    // Extract grid normal from world transform (Y-axis)
    // TODO: can be simpler i.e. u_model[0][1].xyz
    vec3 grid_normal = normalize(u_model[0][1].xyz);
    // Calculate opacity falloff based on distance to grid extents and gracing angle
    vec3 view_dir = normalize(v_view_dir);

    // how much the vector from the camera to the current grid point in space is along grid up i.e. how much we are facing the grid
    // apply an inverted power function to it and invert i.e. more it is align the highest the alpha value will be --> more opaque
    float op_gracing = 1.f - pow(1.f - abs(dot(view_dir, grid_normal)), 16);

    float op_distance = 0.f;
    // distance from projected camera position onto the grid plane
    if (distance_fade_mode == 0.f)
    {
        vec3 view_dir_on_plane = v_view_dir - dot(v_view_dir, grid_normal) * grid_normal;
        float distance_from_camera = length(view_dir_on_plane);
        // TODO: try something more sophisticated than a lerp on the half grid size
        op_distance = (1.f - saturate(distance_from_camera / grid_size));

    }
    // distance from grid center
    else {
        op_distance = (1.f - saturate(length(uv) / grid_size));
    }

    float op = op_distance * op_gracing;

    // Blend between LOD level alphas and scale with opacity falloff. [6]
    color.a *= (thick_cs_a > 0.f ? thick_cs_a : mid_cs_a > 0.f ? mid_cs_a : (thin_cs_a * saturate(1.f-lod_fade))) * op;

    // Visualize LOD levels with different colors (for debugging)
    if (dbg_viz == 1.f)
    {
        if (thick_cs_a > 0.f) color.rgb = vec3(thick_cs_a, 0.f, 0.f);  // Red for thick
        else if (mid_cs_a > 0.f) color.rgb = vec3(0.f, mid_cs_a, 0.f);  // Green for mid
        else if (thin_cs_a > 0.f) color.rgb = vec3(0.f, 0.f, thin_cs_a);  // Blue for thin
    }

    gl_FragColor = color;
}
