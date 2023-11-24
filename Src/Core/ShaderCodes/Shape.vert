#version 450

layout(location = 0) vec3 position;
layout(location = 1) vec3 color;

layout(location = 0) out fragColor;

layout(push_constant) uniform Push {
    mat4 transformMatrix;
    uint sType;
    vec2 resolution;
    vec3 color;
} push;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
} cbo;

void main()
{
    gl_Position = cbo.proj * cbo.view * push.transformMatrix * vec4(position, 1.0);
    fragColor = color;
}