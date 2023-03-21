#version 450

layout(location = 0) out outColor

layout(binding = 0) uniform LightBufferObject {
    mat4 view;
} lbo;

void main()
{
    vec3 lightPosition = vec3(lbo.view[0].x, lbo.view[1].y, lbo.view[2].z);
    outColor = fragColor;
}