#version 450
#extension GL_ARB_separate_shader_objects : enable
 layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
 
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec4 viewPos;
layout(location = 3) out mat3 TBN;

layout(push_constant) uniform Camera
{
    vec4 pos;
    mat4 viewProj;
}camera;

layout(binding = 0) uniform Model_Matrix
{
    mat4[100] matrices;
}models;

layout(binding = 1) uniform Normal_Matrix
{
    mat4[100] matrices;
}normals;

void main() 
{
    fragPos = vec3(models.matrices[gl_InstanceIndex] * vec4(inPosition, 1.0));
    gl_Position = camera.viewProj * vec4(fragPos, 1.0);
    texCoord = inTexCoord;
    viewPos = camera.pos;
    vec3 T = vec3(normalize(normals.matrices[gl_InstanceIndex] * vec4(inTangent, 1.0)));
    vec3 N = vec3(normalize(normals.matrices[gl_InstanceIndex] * vec4(inNormal, 1.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = inBitangent;
    TBN = transpose(mat3(T, B, N));
}