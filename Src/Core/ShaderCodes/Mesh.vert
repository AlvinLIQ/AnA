#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

#ifdef PER_PRIMITIVE_NORMAL
struct VertexOutput {
    vec2 texCoord;
    uint texID;
    vec3 vertexPosition;
    vec3 color;
};
layout(location = 4) perprimitiveEXT out vec3 primitiveNormal;
#else
struct VertexOutput {
    vec2 texCoord;
    uint texID;
    vec3 normalSpace;
    vec3 vertexPosition;
    vec3 color;
};
#endif

layout(location = 0) out VertexOutput vertexOutput;

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
    Vertex vertex = mesh.vertexPtr.vertices[gl_VertexIndex];
    gl_Position = miscPtr.viewProj * vec4(vertex.position, 1.0);
    vertexOutput.color = vertex.color;
    vertexOutput.texCoord = vertex.uv;
    vertexOutput.texID = mesh.textureId;
    vec3 normalSpace =
        normalize(transpose(mat3(mesh.transform)) * CalculateNormal(vertex.pitch, vertex.yaw));
#ifdef PER_PRIMITIVE_NORMAL
    primitiveNormal = normalSpace;
#else
    vertexOutput.normalSpace = normalSpace;
#endif
}
