#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform sampler2D textures[10];

struct Material
{
    uint diffuse;
    uint normal;
    uint speclular;
    uint padding;
};
   
layout(binding = 4) uniform Materials
{
    Material[100] values;
}materials;
 
layout( push_constant ) uniform PushConstant{ 
    layout(offset = 4)uint index;
} materialIndex;

void main() {
    outColor = fragColor * texture(textures[materials.values[materialIndex.index].diffuse], texCoord);
}