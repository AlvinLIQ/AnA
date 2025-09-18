#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoSampler;
layout(set = 0, binding = 3) uniform sampler2D depthSampler;

void main()
{
    outColor = texture(albedoSampler, inUV);
}