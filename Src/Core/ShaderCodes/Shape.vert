#version 460
#extension GL_EXT_scalar_block_layout: enable

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out uint texIndex;
layout(location = 3) out uint texLayer;

struct Shape
{
    mat4 transform;
    vec4 color;
    uint texLayer;
};

layout(scalar, set = 0, binding = 0) buffer SSBO
{
    Shape shapes[];
} ssbo;

const vec2 vertices[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0)
);

const vec2 uvs[] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

void main()
{
    uint index = gl_VertexIndex / 6;
    uint vIndex = gl_VertexIndex % 6;
    gl_Position = ssbo.shapes[index].transform * vec4(vertices[vIndex], 0.0, 1.0);
    fragColor = ssbo.shapes[index].color;
    texIndex = index;
    texCoord = uvs[vIndex];
    texLayer = ssbo.shapes[index].texLayer;
}