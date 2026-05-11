#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec4 position;
    vec2 resolution;
} cbo;

struct Object {
    mat4 model;
};

void main()
{
    gl_PointSize = 10;
    gl_Position = cbo.proj * cbo.view * vec4(position, 1.0);
    fragColor = color;
}
