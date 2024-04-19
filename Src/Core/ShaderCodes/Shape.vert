#version 460

layout(location = 0) out vec3 fragColor;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    uint texIndex;
};

layout(std140, set = 0, binding = 0) buffer VertexSSBO
{
    Vertex vertices[];
} ssbo;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;


void main()
{
    gl_Position = push.transform * vec4(ssbo.vertices[gl_VertexIndex].position, 1.0);
    fragColor = push.color;
}