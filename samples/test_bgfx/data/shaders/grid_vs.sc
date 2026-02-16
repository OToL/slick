$input a_position
$output v_view_dir, v_grid_position

#include <bgfx.sh>

// Uniforms
uniform vec4 u_grid_scalar_params;

void main() {
    float grid_half_size = u_grid_scalar_params.x;

    vec3 local_pos = a_position * grid_half_size;
    vec4 world_pos = mul(u_model[0], vec4(local_pos, 1.0));

    // bgfx is transposing the view matrix and its inverse to ...
    // - support D3D (row-major clip space)
    // - support GL/VK (column-major clip space)
    // - unify depth ranges
    // - avoid rewriting every shader per backend
    // So it chooses to:
    // - canonicalize view/proj on upload
    // - keep model matrices untouched
    // - expose a consistent shader interface
    vec3 camera_world_pos = vec3(u_invView[0][3], u_invView[1][3], u_invView[2][3]);

    v_view_dir = camera_world_pos - world_pos.xyz;
    v_grid_position = local_pos.xz;

    gl_Position = mul(u_viewProj, world_pos);
}
