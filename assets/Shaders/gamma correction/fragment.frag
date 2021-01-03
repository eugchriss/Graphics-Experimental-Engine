#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 finalColor;
layout(location = 0) in vec2 texCoord;

layout(binding = 0) uniform sampler2D input_outColor;

void main()
{    
    finalColor = texture(input_outColor, texCoord);
    finalColor = pow(finalColor, vec4(1.0/2.2));
}