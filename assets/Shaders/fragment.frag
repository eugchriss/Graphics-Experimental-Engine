#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec4 viewPos;
layout(location = 4) in mat3 TBN;
layout(location = 7) in flat uint drawableIndex;

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


layout(binding = 6) uniform DrawableMaterial
{
    uvec4[100] values;
}drawableMaterials;

vec4 computePointLight(Material material, PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() 
{
    uint materialIndex = drawableMaterials.values[drawableIndex / 4][drawableIndex % 4];
    Material material = materials.values[materialIndex];
    vec3 normal = texture(textures[material.normal], texCoord).rgb;
    vec3 viewDir = normalize(viewPos.xyz - fragPos.xyz);
    normal = 2.0 * normal - vec3(1.0);
    outColor = computePointLight(material, pointLights.values[0], normal, fragPos, viewDir);

    //outColor = texture(textures[material.diffuse], texCoord);
    outColor = pow(outColor, vec4(1.0/2.2));
}

vec4 computePointLight(Material material, PointLight light, vec3 normal, vec3 pos, vec3 viewDir)
{
    vec3 tangentLightPos = TBN * light.position.xyz;
    vec3 tangentFragPos = TBN * pos;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // diffuse shading
    float diff = max(dot(lightDir, normal), 0.0);
    
    // specular shading
    float spec = pow(max(dot(normal.xyz, halfwayDir), 0.0), 8.0);
    
    // attenuation
    float distance    = length(light.position.xyz - pos);
    float attenuation = 1.0 / (1 + light.linear * (distance / pow(distance, 1.2) ) + light.quadratic * distance);    
    
    // combine results
    vec4 ambient  = light.ambient * vec4(texture(textures[material.diffuse], texCoord));
    vec4 diffuse  = light.diffuse  * diff * vec4(texture(textures[material.diffuse], texCoord));
    vec4 specular = light.specular * spec * vec4(texture(textures[material.specular], texCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 