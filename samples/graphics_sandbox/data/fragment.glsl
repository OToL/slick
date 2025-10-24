#version 330 core

in vec3 v_view_dir;
in vec2 v_grid_position; // fragment position in grid space [-grid_size; gridsize]

// Output
out vec4 finalColor;

// Uniforms

void main() {
  // Simple animated color based on UV and time
  // vec2 uv = fragTexCoord;
  // vec3 color = vec3(
  //     sin(time + uv.x * 3.14159) * 0.5 + 0.5,
  //     cos(time + uv.y * 3.14159) * 0.5 + 0.5,
  //     sin(time * 2.0) * 0.5 + 0.5
  // );
 
  // vec3 norm_view_dir = normalize(v_view_dir) * vec3(v_grid_position, 1.0);
  vec3 norm_view_dir = normalize(v_view_dir) ;
  vec2 remapped = vec2(20) - v_grid_position;
  norm_view_dir = vec3((normalize(remapped)), 0.0);

  finalColor = vec4(norm_view_dir, 1.0);
  // finalColor = vec4(0.5, 0.5, 0.5 , 1.0);
}
