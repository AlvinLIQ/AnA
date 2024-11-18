#version 460

layout(location = 0) in vec2 texCoord;
layout(set = 4, binding = 0) uniform sampler2D shadowSampler;

void main()
{
    float alpha = texture(shadowSampler, texCoord).a;
    if (alpha < 0.5)
        discard;
}