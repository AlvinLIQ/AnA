#version 460
#extension GL_EXT_mesh_shader : require

layout (location = 0) in VertexInput {
  vec4 color;
} vertexInput;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vertexInput.color;
}