#version 460

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));

void main()
{
    gl_Position = cbo.proj * (vec4(LIGHT_DIRECTION, 1.0) * (objectBuffer.objects[gl_BaseInstance].model * vec4(position, 1.0)));
}