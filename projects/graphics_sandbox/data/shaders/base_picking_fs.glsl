#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input rendering uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Input utility uniforms
uniform ivec2 u_picking_coord;
uniform uvec2 u_obj_identity;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

// gl_FragCoord.xyz : pixel coord and depth
void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    if (u_picking_coord.x == gl_FragCoord.x && u_picking_coord.y == gl_FragCoord.y)
    {
        finalColor = vec4(1, 0, 0, 1);
    }

    finalColor = texelColor*colDiffuse*fragColor;
}


