#version 330 core


precision mediump float; 
in vec3 texCoord_v; 

uniform samplerCube cubemapSampler; 

out vec4 FragColor;

void main() { 
    FragColor = texture(cubemapSampler,texCoord_v); 
} 
