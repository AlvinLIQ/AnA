#version 450


layout(location = 0) out vec4 outColor;

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
    vec3 lightPosition = vec3(cbo.view[0].x, cbo.view[1].y, cbo.view[2].z);
    outColor = vec4(lightPosition, 1.f);
}