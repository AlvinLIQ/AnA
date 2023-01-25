#version 450

layout(location = 0) in vec2 position;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    gl_Position = vec4(push.transform * position, 0.0, 1.0);
}