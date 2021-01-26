#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D textures[10];

void main() 
{
    outColor = texture(textures[0], texCoord);
}