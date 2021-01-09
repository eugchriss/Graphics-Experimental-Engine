#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 texCoord;

layout(binding = 0) uniform Camera
{
    vec4 pos;
    mat4 viewProj;
}camera;	

void main() 
{
    gl_Position = vec4(mat3(camera.viewProj) * inPosition, 1.0);
    texCoord = inPosition;
}