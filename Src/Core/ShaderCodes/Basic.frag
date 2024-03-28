#version 460

#define ANA_MODEL 1
#define ANA_TRIANGLE 2
#define ANA_RECTANGLE 4
#define ANA_ELLIPSE 8
#define ANA_CURVED_RECTANGLE 16
#define ANA_TEXT 32
#define ANA_SPHERE 64

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in uint objectIndex;
layout(location = 3) in vec3 normalSpace;
layout(location = 4) in vec3 vertex;
layout(location = 5) in vec4 shadowCoord;


layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    uint sType;
    vec3 color;
} push;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

layout(set = 3, binding = 0) uniform sampler2D texSampler;
layout(set = 4, binding = 0) uniform sampler2D shadowSampler;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

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
    if (push.sType == ANA_MODEL)
    {
        float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vertex)), 0);
        float diffuseLightItensity = max(dot(normalSpace, normalize(lbo.direction)), 0);
        float visibility = 1.0;
        vec3 shadowProj = vec3(shadowCoord.xyz / shadowCoord.w);
        if (texture(shadowSampler, shadowProj.xy).r < shadowProj.z)
        {
            visibility = 0.3;
        }
        vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient + pointLightIntensity * LIGHT_COLOR) * visibility;
        outColor = texture(texSampler, texCoord) * vec4(vec3(finalLight), 1.0);
        return;
    }
}