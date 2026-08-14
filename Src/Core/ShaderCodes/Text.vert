#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

layout(location = 0) out vec3 outColor;
layout(location = 1) flat out vec4 scissor;

layout(scalar, buffer_reference) buffer VertexRef
{
    vec2 vertices[];
};

struct CharacterInfo
{
    uint8_t ch;
    uint index;
};

layout(scalar, buffer_reference) buffer CharacterInfoRef
{
    CharacterInfo charInfos[];
};

struct TextData
{
    float size;
    vec2 offset;
    vec3 color;
    vec4 scissor;
    uint chOffset;
    uint count;
};

layout(scalar, buffer_reference) buffer TextDataRef
{
    TextData textInfos[];
};

layout(scalar, buffer_reference) buffer MeshletRef
{
    Meshlet meshlets[];
};

layout(scalar, buffer_reference) buffer MeshIndexRef
{
    uint8_t indices[];
};

layout(push_constant) uniform PushConstants
{
    VertexRef vertexPtr;
    CharacterInfoRef charInfoPtr;
    TextDataRef textDataPtr;
    MeshletRef meshletPtr;
    MeshIndexRef meshIndexPtr;
    vec2 resolution;
};

const vec2 charSize = vec2(0.836363613, 0.99999994);

void main()
{
    TextData textInfo = textDataPtr.textInfos[gl_InstanceIndex];
    CharacterInfo chInfo = charInfoPtr.charInfos[gl_DrawID];
    Meshlet meshlet = meshletPtr.meshlets[uint(chInfo.ch)];
    vec2 scale = vec2(textInfo.size / resolution.x, textInfo.size / resolution.y);
    vec2 pos = scale * charSize * vertexPtr.vertices[meshlet.vertexOffset + gl_VertexIndex];
    pos.x += scale.x * charSize.x * float(chInfo.index) - 1.;
    pos.y = -pos.y - charSize.y + scale.y;

    gl_Position = vec4(pos + textInfo.offset, 0.0, 1.0);
    outColor = textInfo.color;
    scissor = textInfo.scissor;
}
