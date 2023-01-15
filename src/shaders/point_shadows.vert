#version 330 core
// Some parts of the code were taken from https://learnopengl.com/

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform bool reverse_normals;

void main()
{

    vs_out.FragPos = vec3(M * vec4(position, 1.0));
    if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
    vs_out.Normal = transpose(inverse(mat3(M))) * (-1.0 * normal);
    else
    vs_out.Normal = transpose(inverse(mat3(M))) * normal;
    vs_out.TexCoords = tex_coords;
    gl_Position = P * V * M * vec4(position, 1.0);
}