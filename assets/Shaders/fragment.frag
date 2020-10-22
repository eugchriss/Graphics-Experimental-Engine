#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 fragPos;
layout(location = 3) in vec4 viewPos;
layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform sampler2D textures[10];

struct Material
{
    uint diffuse;
    uint normal;
    uint specular;
    uint padding;
};
   
layout(binding = 4) uniform Materials
{
    Material[100] values;
}materials;
 
struct PointLight
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float linear;
    float quadratic;
};

layout(binding = 5) uniform PointLights
{
    PointLight[100] values;
}pointLights;
 
layout( push_constant ) uniform PushConstant{ 
    layout(offset = 4)uint index;
} materialIndex;


vec4 computePointLight(PointLight light, vec4 normal, vec4 fragPos, vec4 viewDir);

void main() 
{
    vec4 normal = texture(textures[materials.values[materialIndex.index].normal], texCoord);
    normal = normalize(normal);
    vec4 viewDir = normalize(viewPos - fragPos);

    outColor = computePointLight(pointLights.values[0], normal, fragPos, viewDir);
}

vec4 computePointLight(PointLight light, vec4 normal, vec4 pos, vec4 viewDir)
{
    vec4 lightDir = normalize(light.position - pos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // attenuation
    float distance    = length(light.position - pos);
    float attenuation = 1.0 / (1 + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec4 ambient  = light.ambient  * vec4(texture(textures[materials.values[materialIndex.index].diffuse], texCoord));
    vec4 diffuse  = light.diffuse  * diff * vec4(texture(textures[materials.values[materialIndex.index].diffuse], texCoord));
    vec4 specular = light.specular * spec * vec4(texture(textures[materials.values[materialIndex.index].specular], texCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 