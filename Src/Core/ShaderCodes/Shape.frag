#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec3 outColor;

layout(push_constant) uniform Push {
    mat4 transformMatrix;
    uint sType;
    vec2 resolution;
    vec3 color;
} push;

void main()
{
    outColor = fragColor;
}