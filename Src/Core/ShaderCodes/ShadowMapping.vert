#version 460
/*
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint texIndex;*/

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

const mat4 biasMat = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );

const vec3 LIGHT_DIRECTION = vec3(1., -3., 1.);
void main()
{
    //mat4 dView = mat4(0.999949, -0.009408, 0.003682, 0.000000, 0.000000, 0.364459, 0.931219, 0.000000, -0.010103, -0.931172, 0.364441, 0.000000, -1.931544, -0.269233, 11.256238, 1.000000);
    gl_Position = lbo.proj * lbo.view * (vec4(ssbo.vertices[gl_VertexIndex].position, 1.0));
}