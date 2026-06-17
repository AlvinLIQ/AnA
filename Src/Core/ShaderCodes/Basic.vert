#version 460
#extension GL_EXT_scalar_block_layout: require
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"
#include "bindings.h"

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out uint outTexID;
layout(location = 2) out vec3 outNormalSpace;
layout(location = 3) out vec3 outVertex;
layout(location = 4) out float outViewPosZ;

layout(scalar, set = DEFAULT_VERTEX_LAYOUT, binding = BIND_VERTEX) buffer VertexSSBO
{
    Vertex vertices[];
};

layout(std430, set = DEFAULT_OBJECT_LAYOUT, binding = BIND_OBJECT) buffer ObjectSSBO
{
    Object objects[];
};

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

struct Ray {
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));

const mat4 biasMat = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0
    );

mat4 transform(vec3 scale, vec3 rotation, vec3 transition)
{
    float c3 = cos(rotation.z);
    float s3 = sin(rotation.z);
    float c2 = cos(rotation.x);
    float s2 = sin(rotation.x);
    float c1 = cos(rotation.y);
    float s1 = sin(rotation.y);
    return mat4(
        scale.x * (c1 * c3 + s1 * s2 * s3),
        scale.x * (c2 * s3),
        scale.x * (c1 * s2 * s3 - c3 * s1),
        0.0f,
        scale.y * (c3 * s1 * s2 - c1 * s3),
        scale.y * (c2 * c3),
        scale.y * (c1 * c3 * s2 + s1 * s3),
        0.0f,
        scale.z * (c2 * s1),
        scale.z * (-s2),
        scale.z * (c1 * c2),
        0.0f,
        vec4(transition, 1.0)
    );
}

void main() {
    mat4 transform = objects[gl_DrawID].transform;
    Vertex vertex = vertices[gl_VertexIndex];
    vec4 vertexPos = transform * vec4(vertex.position, 1.0);
    vec4 viewPos = cbo.view * vertexPos;
    gl_Position = cbo.proj * viewPos;
    outNormalSpace = CalculateNormal(vertex.pitch, vertex.yaw);
    outVertex = vertexPos.xyz;
    outViewPosZ = viewPos.z / viewPos.w + ubo.cascades[1].split - ubo.cascades[0].split + 1.0;

    outTexCoord = vec2(vertex.uv);
    outTexID = uint(vertex.texureId);
}
