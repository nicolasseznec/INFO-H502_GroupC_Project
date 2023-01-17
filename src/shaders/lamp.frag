#version 400 core
// Some parts of the code were taken from https://learnopengl.com/

precision mediump float; 
in vec3 v_frag_coord; 
in vec3 v_normal; 
in vec2 v_tex;

out vec4 FragColor;

uniform sampler2D u_texture;
uniform bool enabled;

void main() { 
    vec4 texColor = texture(u_texture, v_tex);
    vec3 color;
    if (enabled) color = vec3(1.0);
    else color = vec3(0.0);

    FragColor = vec4(color, 1.0);
} 
