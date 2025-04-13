#version 450

layout(location = 0) out vec3 fragColor;

struct CharacterVertex
{
    vec2 position;
    vec3 color;
    uint index;
};

layout(std430, set = 0, binding = 0) buffer CharacterVertexSSBO
{
    CharacterVertex vertices[];
};

struct Character
{
    vec2 baseSize;
};

layout(std430, set = 0, binding = 1) buffer CharacterSSBO
{
    Character characters[];
};

layout(std430, set = 1, binding = 0) buffer CharacterTranformSSBO
{
    mat2 transforms[];
};

void main()
{
    CharacterVertex vertex = vertices[gl_VertexIndex];
    gl_Position = vec4(transforms[gl_VertexIndex] * vertex.position, 0.0, 1.0);
    fragColor = vertex.color;
}