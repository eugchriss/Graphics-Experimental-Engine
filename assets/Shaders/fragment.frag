#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform sampler2D diffuseTex[3];

layout( push_constant ) uniform PushConstant{ 
  uint index; 
} pushConstant; 

void main() {
    outColor = fragColor * texture(diffuseTex[pushConstant.index], texCoord);
}










