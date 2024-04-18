#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint texIndex;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec2 resolution;
} cbo;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;


void main()
{
    gl_Position = push.transform * vec4(position, 1.0);
    fragColor = push.color;
}