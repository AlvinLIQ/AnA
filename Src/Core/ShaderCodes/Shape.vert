#version 450

layout(location = 0) vec3 position;
layout(location = 1) vec3 color;

layout(location = 0) out fragColor;

void main()
{
    gl_Position = vec4(position, 1.0);
    fragColor = color;
}