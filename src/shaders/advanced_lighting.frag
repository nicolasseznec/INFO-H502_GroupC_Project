#version 330 core


out vec4 FragColor;

precision mediump float;
in vec3 v_frag_coord;
in vec3 v_normal;
in vec2 v_tex;

uniform vec3 u_view_pos;

struct Light{
    vec3 light_pos;
};

uniform Light light;
uniform vec3 materialColour;

uniform sampler2D u_texture;


void main() {
    vec3 color = texture(u_texture, v_tex).rgb;
    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(light.light_pos - v_frag_coord);
    vec3 normal = normalize(v_normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(u_view_pos - v_frag_coord);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
    //FragColor = texture(u_texture, v_tex);
}
