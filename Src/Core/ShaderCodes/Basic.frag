#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#include "bindings.h"

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
layout(location = 2) in vec3 normalSpace;
layout(location = 3) in vec3 vertex;
layout(location = 4) in vec3 color;

layout(location = 0) out vec4 outColor;

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
const vec3 LIGHT_COLOR = vec3(0.6, 0.7, 0.5);

void main()
{
    vec4 texColor = texture(texSampler[nonuniformEXT(texIndex)], texCoord);
    if (texColor.a < 0.5)
        discard;

    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_POS - vertex)), 0);
    float diffuseLightItensity = ((dot(normalSpace, normalize(lbo.direction))) + 2.0) * 0.5;

    float visibility = 1.0; //filterPCF(shadowCoord, cascadeIndex);

    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility + pointLightIntensity * LIGHT_COLOR;

    outColor = vec4(color * texColor.xyz * finalLight, texColor.a);
    //outColor = texture(shadowSampler, vec3(gl_FragCoord.xy / cbo.resolution, 2)).r * vec4(1.);
}
