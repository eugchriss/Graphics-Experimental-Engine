#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec4 viewPos;
layout(location = 3) in mat3 TBN;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D textures[10];

struct PointLight
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float linear;
    float quadratic;
    float padding1;
    float padding2;
};

layout(binding = 3) uniform PointLights
{
    PointLight[100] values;
}pointLights;

layout(push_constant) uniform PerMaterial {
	layout(offset = 80) uint diffuse;
	uint normal;
	uint specular;
        uint lightCount;
} material;
vec4 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() 
{
    vec3 normal = texture(textures[material.normal], texCoord).rgb;
    vec3 viewDir = normalize(viewPos.xyz - fragPos.xyz);
    normal = 2.0 * normal - vec3(1.0);
    outColor = vec4(0.0);
    for(int i = 0; i < 1; ++i)
    {
        outColor = computePointLight(pointLights.values[i], normal, fragPos, viewDir);
    } 
    outColor = pow(outColor, vec4(1.0/2.2));
}

vec4 computePointLight(PointLight light, vec3 normal, vec3 pos, vec3 viewDir)
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
    return  ambient + diffuse + specular;
} 