#version 460

layout(location = 0) out vec3 baseColor;

const vec2 positions[] = {{-1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}, {1.0, -1.0}};

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}