#version 450
#extension GL_ARB_separate_shader_objects : enable
 layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
 
layout(location = 0) out vec2 texCoord;

layout(binding = 0) uniform Camera
{
    vec4 pos;
    mat4 viewProj;
}camera;

layout(binding = 1) uniform Model_Matrix
{
    mat4[100] matrices;
}models;

void main() 
{
    gl_Position = camera.viewProj * models.matrices[gl_InstanceIndex] * vec4(inPosition, 1.0);
    texCoord = inTexCoord;
}