#version 460

layout(location = 0) out vec4 outVertexPos;

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

#define SHADOW_MAP_CASCADE_COUNT 3
struct Cascade {
    mat4 viewProj;
    float split;
};
layout (set = 5, binding = 0) uniform UBO {
	Cascade[SHADOW_MAP_CASCADE_COUNT] cascades;
} ubo;

struct Ray{
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
    gl_PointSize = 10;
    Vertex vertex = ssbo.vertices[gl_VertexIndex];
    vec4 vertexPos = vec4(vertex.position, 1.0);
    outVertexPos = vertexPos;
    vec4 viewPos = cbo.view * vec4(vertexPos.x, vertexPos.y + 10.0, vertexPos.zw);
    gl_Position = cbo.proj * viewPos;
}