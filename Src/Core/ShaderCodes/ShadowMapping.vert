#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

#define PI 3.14

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

mat4 lookat(vec3 position, vec3 direction, vec3 up)
{
    vec3 w = normalize(direction);
    vec3 u = normalize(cross(w, up));
    vec3 v = normalize(cross(w, u));
    return mat4(u.x, v.x, w.x, 0.0,
                u.y, v.y, w.y, 0.0,
                u.z, v.z, w.z, 0.0,
                position, 1.0);
}

vec3 inverse(vec3 lightDirection)
{
    return -normalize(lightDirection);
}

const mat4 biasMat = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );

const vec3 LIGHT_DIRECTION = vec3(1., -3., 1.);
void main()
{
    //mat4 dView = mat4(0.999949, -0.009408, 0.003682, 0.000000, 0.000000, 0.364459, 0.931219, 0.000000, -0.010103, -0.931172, 0.364441, 0.000000, -1.931544, -0.269233, 11.256238, 1.000000);
    gl_Position = lbo.proj * lbo.view * (objectBuffer.objects[gl_BaseInstance].model * vec4(position, 1.0));
}