#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoSampler;
layout(set = 0, binding = 3) uniform sampler2D depthSampler;

const vec3 LIGHT_POS = vec3(4., -2., 1.);
const vec3 LIGHT_COLOR = vec3(0.6, 0.7, 0.5);

void main()
{
    vec3 normalSpace = texture(normalSampler, inUV).xyz;
    vec3 vertex = texture(positionSampler, inUV).xyz;
    float ambient = 0.17;
    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_POS)), 0) + ambient;
    outColor = vec4(texture(albedoSampler, inUV).rgb * pointLightIntensity, 1.0);
}