#version 330 core
// Input vertex attributes
in vec3 vertexPosition;

// Uniforms
uniform mat4 u_viewProj;
uniform mat4 u_worldTransform;
uniform int u_gridSize;
uniform vec3 u_cameraWorldPos;

out vec3 v_view_dir;
out vec2 v_grid_position;

void main() {
    // scale the grid corner/vertex by actual grid size
    vec3 local_pos = vertexPosition * u_gridSize;
    vec4 world_pos = u_worldTransform * vec4(local_pos, 1.0);

    v_view_dir = u_cameraWorldPos - world_pos.xyz;
    v_grid_position = local_pos.xz;

    gl_Position = u_viewProj * world_pos;
}
