#version 400 core

precision mediump float; 
in vec2 v_tex;

out vec4 FragColor;
 
uniform sampler2D u_texture;

void main() { 
    vec4 texColor = texture(u_texture, v_tex);
    FragColor = texColor;
} 
