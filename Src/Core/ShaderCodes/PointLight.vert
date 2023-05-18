#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
} cbo;

layout(binding = 1) uniform LightingObject
{
    vec3 lightPos;
}lbo;

layout(push_constant) uniform Push {
    mat4 transformMatrix;
    uint sType;
    vec2 resolution;
    vec3 color;
} push;

void main()
{
    vec3 lightDirection = normalize(lbo.lightPos - position);
    vec3 normalWorldSpace = normalize(mat3(push.transformMatrix)*  normal);
    float lightIntensity = max(dot(normalWorldSpace, lightDirection), 0);

    fragColor = lightIntensity * color + 0.077;
    gl_Position = cbo.proj*  cbo.view*  vec4(position, 1.0);
}