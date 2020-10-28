#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 texCoord;

layout(binding = 1) uniform samplerCube skybox;

void main()
{    
    outColor = texture(skybox, texCoord);
}