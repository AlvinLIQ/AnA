#version 460
#extension GL_EXT_scalar_block_layout: enable

layout(location = 0) out vec3 fragColor;

struct Vertex{
    vec2 position;
};

layout(scalar, set = 0, binding = 0) buffer VertexBufferObject {
    Vertex vertices[];
};

struct CharacterInfo{
    vec2 scale;
    vec2 offset;
    vec3 color;
};

layout(std140, set = 1, binding = 0) buffer CharacterInfoBufferObject {
    CharacterInfo charInfos[];
};

void main()
{
    gl_PointSize = 4;
    CharacterInfo charInfo = charInfos[gl_InstanceIndex];
    gl_Position = vec4(charInfo.scale * vertices[gl_VertexIndex].position + charInfo.offset, 0.0, 1.0);
    fragColor = charInfo.color;
}