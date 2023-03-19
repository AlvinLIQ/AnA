#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 uv;

layout(binding = 0) uniform LightBufferObject {
    mat4 view;
} lbo;

void main()
{
    gl_Position = vec4(lbo.view.xyz.xyz * uv * position, lbo.view.z);
}