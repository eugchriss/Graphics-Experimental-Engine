#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out flat uint index;

layout(push_constant) uniform Camera
{
    vec4 pos;
    mat4 viewProj;
}camera;	

layout(binding = 0) uniform Model_Matrix
{
    mat4[100] matrices;
}models;

void main() {
    mat4 modelMatrix = models.matrices[gl_InstanceIndex];
    gl_Position = camera.viewProj * modelMatrix * vec4(inPosition, 1.0);
    //+1 to make sure 0 means no object
    index = gl_InstanceIndex + 1;
}