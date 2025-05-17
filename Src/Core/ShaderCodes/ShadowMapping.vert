#version 460

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

layout(std430, set = 0, binding = 0) buffer VertexSSBO
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

const mat4 biasMat = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );

const vec3 LIGHT_DIRECTION = vec3(1., -3., 1.);
void main()
{
    gl_Position = lbo.proj * lbo.view * (vec4(ssbo.vertices[gl_VertexIndex].position, 1.0));
}