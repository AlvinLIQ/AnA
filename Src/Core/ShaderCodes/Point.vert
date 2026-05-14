#version 460
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"
#include "bindings.h"

layout(location = 0) out vec4 outVertex;
layout(location = 1) out uint vertexIndex;
layout(location = 2) out float intensity;

layout(std430, set = DEFAULT_VERTEX_LAYOUT, binding = BIND_VERTEX) buffer VertexSSBO
{
    float vertices[];
};

layout(set = DEFAULT_UBO_LAYOUT, binding = 0) buffer CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec4 position;
    vec2 resolution;
    vec2 cursorPosition;
    uint selectedIndex;
} cbo;

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
    gl_PointSize = 4;
    uint vertexOffset = gl_VertexIndex * 8;
    vec4 vertexPos = vec4(vertices[vertexOffset + 0], vertices[vertexOffset + 1], vertices[vertexOffset + 2], 1.0);
    vec4 finalPos = cbo.proj * cbo.view * vertexPos;

    gl_Position = finalPos;
    intensity = vertices[vertexOffset + 3];
    vertexIndex = gl_VertexIndex;
    outVertex = vertexPos;
}
