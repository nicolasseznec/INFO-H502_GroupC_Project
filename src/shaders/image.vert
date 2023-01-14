#version 330 core


layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 tex_coords; 
layout (location = 2) in vec3 normal; 
layout (location = 3) in vec3 tangent; 
layout (location = 4) in vec3 bitangent; 

out vec2 v_tex;

void main() { 
    gl_Position = vec4(-position.z*0.75, -position.x*0.75, position.y, 1.0); 
    v_tex = tex_coords;
}
