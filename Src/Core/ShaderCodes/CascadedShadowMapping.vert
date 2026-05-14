#version 460
#extension GL_EXT_scalar_block_layout: enable
#extension GL_ARB_shader_viewport_layer_array: enable
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

layout(scalar, set = DEFAULT_VERTEX_LAYOUT, binding = BIND_VERTEX) buffer VertexSSBO
{
    Vertex vertices[];
} ssbo;

layout(set = DEFAULT_UBO_LAYOUT, binding = 0) buffer CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec4 position;
    vec2 resolution;
} cbo;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

#define SHADOW_MAP_CASCADE_COUNT 2
struct Cascade {
    mat4 viewProj;
    float split;
};

layout(set = 5, binding = 0) uniform UBO {
    Cascade[SHADOW_MAP_CASCADE_COUNT] cascades;
} ubo;

layout(push_constant) uniform PushConsts {
    uint cascadeIndex;
} push;

void main()
{
    Vertex vertex = ssbo.vertices[gl_VertexIndex];
    vec4 vertexPos = vec4(vertex.position, 1.0);
    gl_Position = ubo.cascades[push.cascadeIndex].viewProj * vertexPos;
    gl_Layer = cascadeIndex;
}
