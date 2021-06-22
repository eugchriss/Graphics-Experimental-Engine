#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;


layout(push_constant) uniform PushConsts
{
   layout(offset = 64) uint value;
} material_index;

layout(binding = 1) uniform sampler2D colors;
layout(binding = 2) uniform sampler2D normals[15];



void main() {
    outColor = texture(colors, texCoord);
}