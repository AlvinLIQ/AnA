#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
//    mat4 projectionMatrix;
    mat4 transformMatrix;
    uint sType;
    vec2 resolution;
    vec3 color;
} push;

layout(binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
} cbo;

void main() {
    //gl_Position = vec4(push.transform * position + push.offset * 2, 0.0, 1.0);
    gl_Position = cbo.proj * cbo.view * push.transformMatrix * vec4(position, 1.0);
    fragColor = color;
    //gl_Position = push.projectionMatrix * push.transformMatrix * vec4(position, 1.0);
}