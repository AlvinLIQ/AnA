#version 460
#extension GL_EXT_mesh_shader: require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
#ifdef PER_PRIMITIVE_NORMAL
layout(location = 2) in vec3 vertex;
layout(location = 3) in vec3 color;
layout(location = 4) perprimitiveEXT in vec3 normalSpace;
#else
layout(location = 2) in vec3 normalSpace;
layout(location = 3) in vec3 vertex;
layout(location = 4) in vec3 color;
#endif
#ifdef DEFERRED
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
#else
layout(location = 0) out vec4 outColor;
#endif

const vec3 LIGHT_POS = vec3(4., 2., 1.);
const vec3 LIGHT_COLOR = vec3(1.0, 1.0, 1.0);
const vec3 AMBIENT = vec3(0.033);

void main()
{
    float normalLightPos = dot(normalSpace, normalize(LIGHT_POS));
    float diffuseLightItensity = (normalLightPos + 2.0) * 0.5;
    vec3 finalLight = (diffuseLightItensity * LIGHT_COLOR) + AMBIENT;
    outColor = vec4(color * finalLight, 1.0);
}
