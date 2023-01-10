#version 330 core


out vec4 FragColor;

precision mediump float;
in vec3 v_frag_coord;
in vec3 v_normal;
in vec2 v_tex;

uniform vec3 u_view_pos;

struct Light{
    vec3 light_pos;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform Light light;
uniform vec3 materialColour;

uniform sampler2D u_texture;

uniform float shininess;


void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(u_texture, v_tex).rgb;

    // diffuse
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(light.light_pos - v_frag_coord);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(u_texture, v_tex).rgb;

    // specular
    vec3 viewDir = normalize(u_view_pos - v_frag_coord);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * texture(u_texture, v_tex).rgb;

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    // attenuation
    float distance = length(light.light_pos - v_frag_coord);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}