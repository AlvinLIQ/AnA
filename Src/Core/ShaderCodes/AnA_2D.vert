#version 450

layout(location = 0) in vec2 position;

layout(push_constant) uniform Push {
    mat2 transform;
    uint sType;//0 Triangle 1 Rectangle 2 Circle
    vec2 offset;
    vec2 size;
    vec2 resolution;
    vec3 color;
} push;

void main() {
    gl_Position = vec4(push.transform * position, 0.0, 1.0);
}