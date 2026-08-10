#version 450
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

layout(scalar, buffer_reference) buffer VertexRef
{
	Vertex vertices[];
};

layout(scalar, buffer_reference) buffer IndexRef
{
	uint indices[];
};

layout(scalar, buffer_reference) buffer MiscRef
{
	uint objectCount;
	uint collidedCount;
	uint meshletCount;
	uint meshletIDCount;
	mat4 viewProj;
	vec4 planes[6];
	vec2 resolution;
	vec3 cameraPosition;
};

layout(push_constant) uniform PushConstants
{
	VertexRef vertexPtr;
	IndexRef indexPtr;
	MiscRef miscPtr;
};

void main()
{
	gl_Position = miscPtr.viewProj * vec4(vertexPtr.vertices[indexPtr.indices[gl_VertexIndex]], 1.0);
}
