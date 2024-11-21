#version 460
#extension GL_EXT_nonuniform_qualifier : enable 

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
layout(set = 3, binding = 0) uniform sampler2D texSampler[];

void main()
{
    float alpha = texture(texSampler[nonuniformEXT(texIndex)], texCoord).a;
    if (alpha < 0.5)
        discard;
}