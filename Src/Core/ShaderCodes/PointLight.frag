#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
} cbo;

layout(binding = 1) uniform LightingObject
{
    vec3 lightPos;
}lbo;

layout(push_constant) uniform Push {
    mat4 transformMatrix;
    uint sType;
    vec2 resolution;
    vec3 color;
} push;

void main()
{
    outColor = vec4(fragColor, 1.f);
}