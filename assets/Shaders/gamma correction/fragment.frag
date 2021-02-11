#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 texCoord;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput outputTexture;

layout(binding = 1) uniform Exposure
{ 
    float value;
}exposure;

layout(binding = 2) uniform HDR
{ 
    bool value;
}useHdr;

layout(binding = 3) uniform GammaCorrection
{ 
    bool value;
}useGammaCorrection;

void main()
{    
    outputColor = vec4(subpassLoad(outputTexture)); 
    if(useHdr.value)
    {
	outputColor = outputColor / (outputColor + vec4(1.0));
        outputColor = vec4(1.0) - exp(-outputColor * exposure.value);
    }
    if(useGammaCorrection.value)
    {
        outputColor = pow(outputColor, vec4(1.0/2.2));
    }

}