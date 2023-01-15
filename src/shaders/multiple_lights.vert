#version 330 core


layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 M;
uniform mat4 itM;
uniform mat4 V;
uniform mat4 P;

out vec3 v_frag_coord;
out vec3 v_normal;
out vec2 v_tex;
out mat3 v_TBN;

void main() {
    vec4 frag_coord = M*vec4(position, 1.0);
    gl_Position = P*V*frag_coord;
    v_normal = vec3(itM * vec4(normal, 1.0));
    v_frag_coord = frag_coord.xyz;
    v_tex = tex_coords;

    vec3 T = length(tangent) > 0.0 ? normalize(vec3(M * vec4(tangent, 0.0))) : vec3(0.0);
    vec3 B = length(bitangent) > 0.0 ? normalize(vec3(M * vec4(bitangent, 0.0))) : vec3(0.0);
    vec3 N = normalize(vec3(M * vec4(normal, 0.0)));
    v_TBN = mat3(T, B, N);
}
