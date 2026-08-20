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

layout(set = 0, binding = 0) uniform sampler _sampler;
layout(set = 1, binding = 0) uniform texture2D textures[];

const vec3 LIGHT_POS = vec3(4., 2., 1.);
const vec3 LIGHT_COLOR = vec3(1.0, 1.0, 1.0);
const vec3 AMBIENT = vec3(0.13);

void main()
{
    vec4 texColor = texture(sampler2D(textures[nonuniformEXT(texIndex)], _sampler), texCoord);
    if (texColor.a < 0.8)
        discard;

    float normalLightPos = dot(normalSpace, normalize(LIGHT_POS));
    float diffuseLightItensity = (normalLightPos + 2.0) * 0.3;
    vec3 finalLight = (diffuseLightItensity * LIGHT_COLOR) + AMBIENT;
    outColor = vec4(color * finalLight * texColor.rgb, 1.0);
}
