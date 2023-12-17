#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    uint sType;
    vec3 color;
} push;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    Object objects[];
};

void main()
{
    gl_PointSize = 10;
    gl_Position = cbo.proj * cbo.view * objects[gl_BaseInstance].model * vec4(position, 1.0);
    fragColor = color;
}