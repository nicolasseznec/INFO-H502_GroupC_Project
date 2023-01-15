#version 330 core
// Some parts of the code were taken from https://learnopengl.com/


precision mediump float; 
in vec3 texCoord_v; 

uniform samplerCube cubemapSampler; 

out vec4 FragColor;

void main() { 
    FragColor = texture(cubemapSampler,texCoord_v); 
} 
