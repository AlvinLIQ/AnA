#version 460
#extension GL_EXT_nonuniform_qualifier : enable 

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
layout(location = 2) in vec3 normalSpace;
layout(location = 3) in vec3 vertex;
layout(location = 4) in vec4 shadowCoord;


layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
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
layout(set = 4, binding = 0) uniform sampler2D shadowSampler;

struct Ray{
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));
const vec3 LIGHT_COLOR = vec3(0.95, 0.7, 0.95);

vec3 GetPointOfRay(Ray ray, float len)
{
    return ray.center + len * ray.direction;
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
    //float shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0));
    //outColor = shadow * vec4(1.);
	//float shadow = texture(shadowSampler, shadowCoord.xy).r;
    //float lightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vec3(vertex))), 0);
    //outColor = texture(texSampler, texCoord) * (vec4(lightIntensity * LIGHT_COLOR + 0.033, 1.0));

    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vertex)), 0);
    float diffuseLightItensity = max(dot(normalSpace, normalize(lbo.direction)), 0);
    float visibility = 1.0;
    vec3 shadowProj = vec3(shadowCoord.xyz / shadowCoord.w);
    if (texture(shadowSampler, shadowProj.xy).r < shadowProj.z)
    {
        visibility = 0.5;
    }
    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility + pointLightIntensity * LIGHT_COLOR;
    outColor = texture(texSampler[nonuniformEXT(texIndex)], texCoord) * vec4(finalLight, 1.0);
}