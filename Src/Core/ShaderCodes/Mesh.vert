#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

#ifdef PER_PRIMITIVE_NORMAL
layout(location = 0) out vec2 texCoord;
layout(location = 1) out uint texID;
layout(location = 2) out vec3 vertexPosition;
layout(location = 3) out vec3 color;
layout(location = 4) out vec3 primitiveNormal;
#else
layout(location = 0) out vec2 texCoord;
layout(location = 1) out uint texID;
layout(location = 2) out vec3 normalSpace;
layout(location = 3) out vec3 vertexPosition;
layout(location = 4) out vec3 color;

#endif

layout(scalar, buffer_reference) buffer VertexRef
{
    Vertex vertices[];
};

layout(std430, buffer_reference) buffer IndexRef
{
    uint indices[];
};

struct Mesh
{
    vec3 center;
    float radius;
    vec3 halfVolume;
    uint textureId;
    mat4 transform;
    VertexRef vertexPtr;
    uint64_t meshletVertexPtr;
    uint64_t meshletIndexPtr;
    IndexRef indexPtr;
};

layout(scalar, buffer_reference) buffer MeshRef
{
    Mesh meshes[];
};

layout(scalar, buffer_reference) buffer MiscRef
{
    uint objectCount;
    uint collidedCount;
    uint meshletCount;
    uint meshletIDCount;
    mat4 viewProj;
    vec4 planes[6];
    vec2 resolution;
    vec3 cameraPosition;
};

layout(push_constant) uniform PushConstants
{
    MeshRef meshPtr;
    MiscRef miscPtr;
};

void main()
{
    Mesh mesh = meshPtr.meshes[gl_DrawID];
    Vertex vertex = mesh.vertexPtr.vertices[mesh.indexPtr.indices[gl_VertexIndex]];
    vec4 vertexPos = mesh.transform * vec4(vertex.position, 1.0);
    gl_Position = miscPtr.viewProj * vertexPos;
    color = vec3(
        float(vertex.color.r) / 255.0,
        float(vertex.color.g) / 255.0,
        float(vertex.color.b) / 255.0);
    texCoord = vertex.uv;
    texID = mesh.textureId;
    vertexPosition = vertexPos.xyz / vertexPos.w;
    vec3 normal =
        normalize(transpose(mat3(mesh.transform)) * CalculateNormal(vertex.pitch, vertex.yaw));
#ifdef PER_PRIMITIVE_NORMAL
    primitiveNormal = normal;
#else
    normalSpace = normal;
#endif
}
