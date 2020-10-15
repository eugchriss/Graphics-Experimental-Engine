#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 texCoord;

layout(binding = 0) uniform Camera
{
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

void main() {
    gl_Position = camera.proj * camera.view * models.matrices[gl_InstanceIndex] * vec4(inPosition, 1.0);
    fragColor = colors.color[gl_InstanceIndex];
    texCoord = inTexCoord;
}