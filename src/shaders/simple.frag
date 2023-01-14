#version 330 core


out vec4 FragColor;

precision mediump float; 
in vec3 v_frag_coord; 
in vec3 v_normal; 
in vec2 v_tex;
in mat3 v_TBN;

uniform vec3 u_view_pos; 

struct Light{
    vec3 light_pos; 
    float ambient_strength; 
    float diffuse_strength; 
    float specular_strength; 
    float constant;
    float linear;
    float quadratic;
};

uniform Light light;uniform float shininess; 
uniform vec3 materialColour; 

uniform sampler2D u_texture;
uniform sampler2D u_normalMap;

float specularCalculation(vec3 N, vec3 L, vec3 V ) { 
    vec3 R = reflect (-L,N);  
    float cosTheta = dot(R , V); 
    float spec = pow(max(cosTheta,0.0), 32.0); 
    return light.specular_strength * spec;
}

void main() { 
    // vec3 N = normalize(v_normal);
    vec3 normal = texture(u_normalMap, v_tex).xyz;
    vec3 N = normal * 2.0 - 1.0;
    N = normalize(v_TBN * N);
    
    vec3 L = normalize(light.light_pos - v_frag_coord) ; 
    vec3 V = normalize(u_view_pos - v_frag_coord); 
    float specular = specularCalculation( N, L, V); 
    float diffuse = light.diffuse_strength * max(dot(N,L),0.0);
    float distance = length(light.light_pos - v_frag_coord);float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);float light = light.ambient_strength + attenuation * (diffuse + specular); 
    // FragColor = vec4(materialColour * vec3(light), 1.0); 
    

    FragColor = vec4(texture(u_texture, v_tex).xyz * vec3(light)* 1.5, 1.0);
    // FragColor = texture(u_texture, v_tex);
} 
