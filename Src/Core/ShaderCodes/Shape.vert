#version 460

layout(location = 0) out vec3 fragColor;
layout(location = 1) flat out uint texIndex;

struct Shape
{
    mat4 transform;
    vec3 color;
    uint texIndex;
};

layout(std140, set = 0, binding = 0) buffer SSBO
{
    Shape shapes[];
} ssbo;

vec2 vertices[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0)
);

void main()
{
    uint index = gl_VertexIndex / 6;
    gl_Position = ssbo.shapes[index].transform * vec4(vertices[gl_VertexIndex % 6], 0.0, 1.0);
    fragColor = ssbo.shapes[index].color;
    texIndex = ssbo.shapes[index].texIndex;
}