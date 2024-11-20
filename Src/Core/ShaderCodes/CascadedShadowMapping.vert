#version 460

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

#define SHADOW_MAP_CASCADE_COUNT 4

layout (set = 3, binding = 0) uniform UBO {
	mat4[SHADOW_MAP_CASCADE_COUNT] cascadeViewProjMat;
} ubo;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    Vertex vertex = ssbo.vertices[gl_VertexIndex];
    vec4 vertexPos = vec4(vertex.position, 1.0);
    gl_Position = ubo.cascadeViewProjMat[gl_InstanceIndex] * vertexPos;
    outTexCoord = vertex.uv;
}