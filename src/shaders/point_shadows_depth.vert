#version 330 core
// Some parts of the code were taken from https://learnopengl.com/

layout (location = 0) in vec3 position;

uniform mat4 M;

void main()
{
    gl_Position = M * vec4(position, 1.0);
}