#version 400 core

precision mediump float; 
in vec3 v_frag_coord; 
in vec3 v_normal; 
in vec2 v_tex;

out vec4 FragColor;

uniform vec3 u_view_pos; 
uniform sampler2D u_texture;
uniform samplerCube cubemapSampler; 
uniform float refractionIndice;

void main() { 
    float ratio = 1.00 / refractionIndice;
    vec3 N = normalize(v_normal);
    vec3 V = normalize(u_view_pos - v_frag_coord); 
    vec3 R = refract(-V,N,ratio); 

    vec4 cmColor = texture(cubemapSampler,R);
    vec4 texColor = texture(u_texture, v_tex);

    FragColor = cmColor + texColor;
} 
