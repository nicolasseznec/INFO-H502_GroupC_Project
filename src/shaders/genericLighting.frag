#version 330 core

// Some parts of the code were taken from https://learnopengl.com/


out vec4 FragColor;

precision mediump float;
in vec3 v_frag_coord;
in vec3 v_normal;
in vec2 v_tex;
in mat3 v_TBN;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};

struct PointLight {
    vec3 light_pos;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};

struct SpotLight {
    vec3 light_pos;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};

#define NR_DIR_LIGHTS 1
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1


uniform vec3 u_view_pos;

uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform vec3 materialColour;
uniform float shininess;


uniform sampler2D u_texture;

uniform sampler2D u_normalMap;
uniform bool useNormalMap;


//----------------------------------
// Shadow related
uniform samplerCube depthMap;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

uniform float far_plane;
uniform vec3 lightPos;
bool inShadow = false;

float ShadowCalculation(vec3 fragPos);
//----------------------------------


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor);

void main()
{
    // Normal vector can use normal map if available
    vec3 norm;
    if (useNormalMap) {
        norm = texture(u_normalMap, v_tex).xyz * 2.0 - 1.0;
        norm = normalize(v_TBN * norm);
    }
    else {
        norm = normalize(v_normal);
    }

    vec3 viewDir = normalize(u_view_pos - v_frag_coord);
    vec3 texColor = texture(u_texture, v_tex).rgb;

    // Shadows
    float shadow = ShadowCalculation(v_frag_coord) * 0.5;
    if (shadow > 0.1) inShadow = true;

    // directional lighting
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_DIR_LIGHTS; i++) {
        if (!dirLights[i].enabled) continue;
        result += CalcDirLight(dirLights[i], norm, viewDir, texColor);
    }

    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        if (!pointLights[i].enabled) continue;
        result += CalcPointLight(pointLights[i], norm, v_frag_coord, viewDir, texColor);
    }

    // spot lights
    for(int i = 0; i < NR_SPOT_LIGHTS; i++) {
        if (!spotLights[i].enabled) continue;
        result += CalcSpotLight(spotLights[i], norm, v_frag_coord, viewDir, texColor);
    }
   
    // FragColor = vec4(result, 1.0);
    FragColor = vec4(result * (1.0 - shadow), 1.0);
    // FragColor = vec4(vec3(shadow), 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // combine results
    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * texColor;

    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor)
{
    vec3 lightDir = normalize(light.light_pos - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // attenuation
    float distance = length(light.light_pos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // combine results
    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * texColor;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor)
{
    vec3 lightDir = normalize(light.light_pos - fragPos);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // attenuation
    float distance = length(light.light_pos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // combine results
    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * texColor;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    if (inShadow)
        return ambient;
    else
        return (ambient + diffuse + specular);
}

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    // PCF
    // float shadow = 0.0;
    // float bias = 0.05;
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
    // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
    // {
    // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
    // {
    // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
    // closestDepth *= far_plane;   // Undo mapping [0;1]
    // if(currentDepth - bias > closestDepth)
    // shadow += 1.0;
    // }
    // }
    // }
    // shadow /= (samples * samples * samples);

    // float shadow = 0.0;
    // float bias = 0.15;
    // int samples = 5;
    // float viewDistance = length(u_view_pos - fragPos);
    // float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    // for(int i = 0; i < samples; i++)
    // {
    //     float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
    //     closestDepth *= far_plane;   // undo mapping [0;1]
    //     if(currentDepth - bias > closestDepth)
    //     shadow += 1.0;
    // }
    // shadow /= float(samples);


    float closestDepth = texture(depthMap, fragToLight).r * far_plane;
    float bias = 0.05;
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}