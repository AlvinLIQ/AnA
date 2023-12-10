#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec3 outColor;

layout(push_constant) uniform Push {
    uint sType;
    uint index;
    vec3 color;
} push;

void main()
{
    outColor = fragColor;
}