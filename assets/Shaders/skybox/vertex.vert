#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 texCoord;

layout(binding = 0) uniform Camera
{
    vec4 pos;
    mat4 view;
    mat4 proj;
}camera;	

void main() 
{
    gl_Position = camera.proj * camera.view * vec4(inPosition, 1.0);
    texCoord = inPosition;
}