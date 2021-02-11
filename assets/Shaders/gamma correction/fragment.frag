#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 texCoord;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput outputTexture;

void main()
{    
    outputColor = vec4(subpassLoad(outputTexture));
    outputColor = pow(outputColor, vec4(1.0/2.2));
}