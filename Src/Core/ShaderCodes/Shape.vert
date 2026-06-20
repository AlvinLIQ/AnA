#version 460
#extension GL_EXT_scalar_block_layout: enable

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 bounding;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out uint texIndex;
layout(location = 4) out uint texLayer;

struct Shape
{
    vec2 scale;
    vec2 translation;
    vec4 bounding;
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
    gl_Position = vec4(ssbo.shapes[index].scale * vertices[vIndex] + ssbo.shapes[index].translation, 0.0, 1.0);
    fragColor = ssbo.shapes[index].color;
    bounding = ssbo.shapes[index].bounding;
    texIndex = index;
    texCoord = uvs[vIndex];
    texLayer = ssbo.shapes[index].texLayer;
}
