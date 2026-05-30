#version 460
#extension GL_EXT_mesh_shader: require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "mesh.h"
#include "bindings.h"

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

layout(set = DEFAULT_UBO_LAYOUT, binding = 0) buffer CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec4 position;
    vec2 resolution;
} cbo;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

layout(set = 3, binding = 0) uniform sampler2D texSampler[];
layout(set = 4, binding = 0) uniform sampler2DArray shadowSampler;

#define SHADOW_MAP_CASCADE_COUNT 2
struct Cascade {
    mat4 viewProj;
    float split;
};
layout(set = 5, binding = 0) uniform UBO {
    Cascade[SHADOW_MAP_CASCADE_COUNT] cascades;
} ubo;

const vec3 LIGHT_POS = vec3(4., 2., 1.);
const vec3 LIGHT_COLOR = vec3(1.0, 1.0, 1.0);

void main()
{
    #ifdef DEFERRED
    outPosition = vec4(vertex, 1.0);
    outNormal = vec4(normalSpace, 1.0);
    outAlbedo = texture(texSampler[nonuniformEXT(texIndex)], texCoord);
    #else
    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_POS - vertex)), 0);
    float normalLightPos = dot(normalSpace, normalize(lbo.direction));
    float diffuseLightItensity = (normalLightPos + 2.0) * 0.5;

    float visibility = 1.0; // filterPCF(shadowCoord, cascadeIndex);

    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility; // + pointLightIntensity * LIGHT_COLOR;
    outColor = vec4(color, 1.0) * texture(texSampler[nonuniformEXT(texIndex)], texCoord) * vec4(finalLight, 1.0);
    #endif
}
