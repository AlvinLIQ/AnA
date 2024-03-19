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
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 1) uniform sampler2D shadowSampler;

struct Ray{
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));
const vec3 LIGHT_COLOR = vec3(0.9, 0.9, 0.2);

vec3 GetPointOfRay(Ray ray, float len)
{
    return ray.center + len * ray.direction;
}

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowSampler, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowSampler, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main()
{
    //float shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0));
    //outColor = shadow * vec4(1.);
    float lightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vec3(vertex))), 0);
    outColor = texture(texSampler, texCoord) * (vec4(lightIntensity * LIGHT_COLOR + 0.033, 1.0));
}