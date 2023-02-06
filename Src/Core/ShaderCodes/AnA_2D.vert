#version 450

layout(location = 0) in vec3 position;

layout(push_constant) uniform Push {
    mat4 transform;
    uint sType;//0 Triangle 1 Rectangle 2 Circle
    vec2 resolution;
    vec3 color;
} push;

void main() {
    //gl_Position = vec4(push.transform * position + push.offset * 2, 0.0, 1.0);
    gl_Position = push.transform * vec4(position, 1.0);
}