#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec4 viewPos;
layout(location = 4) out mat3 TBN;

layout(binding = 0) uniform Camera
{
    vec4 pos;
    mat4 view;
    mat4 proj;
}camera;	

layout(binding = 1) uniform Model_Matrix
{
    mat4[100] matrices;
}models;

layout(binding = 2) uniform Colors
{
     vec4[100] color;
}colors;

layout( push_constant ) uniform PushConstant{ 
    uint index;
} modelIndex; 

void main() {
    mat4 modelMatrix = models.matrices[modelIndex.index + gl_InstanceIndex];
    fragPos = vec3( modelMatrix * vec4(inPosition, 1.0));
    gl_Position = camera.proj * camera.view * vec4(fragPos, 1.0);
    fragColor = colors.color[modelIndex.index + gl_InstanceIndex];
    
    texCoord = inTexCoord;
    viewPos = camera.pos;
    mat3 normalMatrix = transpose(inverse(mat3(models.matrices[modelIndex.index + gl_InstanceIndex])));
    vec3 T = normalize(normalMatrix * inTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = transpose(mat3(T, B, N));
}