#version 460

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out uint outTexID;
layout(location = 2) out vec3 outNormalSpace;
layout(location = 3) out vec3 outVertex;
layout(location = 4) out vec4 outShadowCoord;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    uint texIndex;
};

layout(std140, set = 0, binding = 0) buffer VertexSSBO
{
    Vertex vertices[];
} ssbo;

layout(set = 1, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec2 resolution;
} cbo;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

struct Ray{
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));

const mat4 biasMat = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );
/*
const mat4 paddingMat = mat4(
    0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.2, -0.2, 0.0, 1.0
);*/

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
    Vertex vertex = ssbo.vertices[gl_VertexIndex];
    vec4 vertexPos = vec4(vertex.position, 1.0);
    gl_Position = cbo.proj * cbo.view * vertexPos;
    outNormalSpace = normalize(vertex.normal);
    outVertex = vertexPos.xyz / vertexPos.w;
    //mat4 dView = mat4(0.999949, -0.009408, 0.003682, 0.000000, 0.000000, 0.364459, 0.931219, 0.000000, -0.010103, -0.931172, 0.364441, 0.000000, -1.931544, -0.269233, 11.256238, 1.000000);
    outShadowCoord = biasMat * lbo.proj * lbo.view * vertexPos;

    outTexCoord = vertex.uv;
    outTexID = vertex.texIndex;
    //gl_Position = push.projectionMatrix * push.transformMatrix * vec4(position, 1.0);
}