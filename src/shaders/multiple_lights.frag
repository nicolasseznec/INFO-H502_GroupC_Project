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
};

struct PointLight {
    vec3 light_pos;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
};

#define NR_DIR_LIGHTS 1
#define NR_POINT_LIGHTS 1
// #define NR_SPOT_LIGHTS 2
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

float ShadowCalculation(vec3 fragPos);
//----------------------------------


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // properties
    vec3 norm;
    if (useNormalMap) {
        norm = texture(u_normalMap, v_tex).xyz * 2.0 - 1.0;
        norm = normalize(v_TBN * norm);

        // FragColor = vec4(norm * 0.5 + 0.5, 1.0);
        // return;
    }
    else {
        norm = normalize(v_normal);
    }

    vec3 viewDir = normalize(u_view_pos - v_frag_coord);

    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_DIR_LIGHTS; i++)
    result += CalcDirLight(dirLights[i], norm, viewDir);
    //vec3 result = CalcDirLight(dirLights, norm, viewDir);


    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    result += CalcPointLight(pointLights[i], norm, v_frag_coord, viewDir);



    // phase 3: spot light
    for(int i = 0; i < NR_SPOT_LIGHTS; i++)
    result += CalcSpotLight(spotLights[i], norm, v_frag_coord, viewDir);
    //result += CalcSpotLight(spotLights, norm, v_frag_coord, viewDir);

    // FragColor = vec4(result, 1.0);





    float shadow = ShadowCalculation(v_frag_coord) * 0.5;
    // float shadow = ShadowCalculation(v_frag_coord);
    FragColor = vec4(result * (1.0 - shadow), 1.0);
    // result = vec3(1.0, 1.0, 1.0)
    // FragColor = vec4(result * shadow, 1.0);
    // FragColor = vec4(result.x * (1.0 - shadow), result.y * (1.0 - shadow), result.z * (1.0 - shadow), 1.0);
    // FragColor = vec4(vec3(shadow), 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(u_texture, v_tex).rgb);
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texture, v_tex).rgb);
    vec3 specular = light.specular * spec * vec3(texture(u_texture, v_tex).rgb);
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    vec3 ambient = light.ambient * vec3(texture(u_texture, v_tex).rgb);
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texture, v_tex).rgb);
    vec3 specular = light.specular * spec * vec3(texture(u_texture, v_tex).rgb);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    vec3 ambient = light.ambient * vec3(texture(u_texture, v_tex).rgb);
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texture, v_tex).rgb);
    vec3 specular = light.specular * spec * vec3(texture(u_texture, v_tex).rgb);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // use the fragment to light vector to sample from the depth map
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
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

    float shadow = 0.0;
    // float bias = 0.15;
    // int samples = 20;
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


    float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= far_plane;
    float bias = 0.05;
    // float bias = 0.005;
    shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;


    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);
    // float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[0] * diskRadius).r;
    // shadow = closestDepth / far_plane;

    return shadow;
}