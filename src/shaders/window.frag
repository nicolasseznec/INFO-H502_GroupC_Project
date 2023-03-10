#version 400 core
// Some parts of the code were taken from https://learnopengl.com/

precision mediump float; 
in vec3 v_frag_coord; 
in vec3 v_normal; 
in vec2 v_tex;

out vec4 FragColor;

uniform vec3 u_view_pos; 
uniform sampler2D u_texture;
uniform samplerCube cubemapSampler; 

void main() { 
    vec4 texColor = texture(u_texture, v_tex);
    float refIndice = texColor.x;

    float ratio = 1.00 / (refIndice*0.5 + 1.0);
    vec3 N = normalize(v_normal);
    vec3 V = normalize(u_view_pos - v_frag_coord); 
    vec3 R = refract(-V,N,ratio); 

    vec4 cmColor = texture(cubemapSampler,R);

    FragColor = cmColor + texColor*0.5;
} 
