#version 400 core
// Some parts of the code were taken from https://learnopengl.com/

precision mediump float; 
in vec3 v_frag_coord; 
in vec3 v_normal; 
in vec2 v_tex;

out vec4 FragColor;

uniform sampler2D u_texture;


void main() { 
    vec4 texColor = texture(u_texture, v_tex);
    FragColor = vec4(1.0);
} 
