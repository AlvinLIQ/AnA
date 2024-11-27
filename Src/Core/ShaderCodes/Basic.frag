#version 460
#extension GL_EXT_nonuniform_qualifier : enable 

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
layout(location = 2) in vec3 normalSpace;
layout(location = 3) in vec3 vertex;
layout(location = 4) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
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

#define SHADOW_MAP_CASCADE_COUNT 4
struct Cascade {
    mat4 viewProj;
    float split;
};
layout (set = 5, binding = 0) uniform UBO {
	Cascade[SHADOW_MAP_CASCADE_COUNT] cascades;
} ubo;

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));
const vec3 LIGHT_COLOR = vec3(0.95, 0.7, 0.95);

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float shadow = 1.0;
	float bias = 0.005;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(shadowSampler, vec3(shadowCoord.st + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 0.5;
		}
	}
	return shadow;

}

float filterPCF(vec4 sc, uint cascadeIndex)
{
	ivec2 texDim = textureSize(shadowSampler, 0).xy;
	float scale = 0.75;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex);
			count++;
		}
	}
	return shadowFactor / count;
}

void main()
{
    //float shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0));
    //outColor = shadow * vec4(1.);
	//float shadow = texture(shadowSampler, shadowCoord.xy).r;
    //float lightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vec3(vertex))), 0);
    //outColor = texture(texSampler, texCoord) * (vec4(lightIntensity * LIGHT_COLOR + 0.033, 1.0));

    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vertex)), 0);
    float diffuseLightItensity = max(dot(normalSpace, normalize(lbo.direction)), 0);
    uint cascadeIndex = 0;
    for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(viewPos.z < ubo.cascades[i].split) {	
			cascadeIndex = i + 1;
		}
	}
    vec4 shadowCoord = (biasMat * ubo.cascades[cascadeIndex].viewProj) * vec4(vertex, 1.0);	
    float visibility = 1.0f; //filterPCF(shadowCoord / shadowCoord.w, cascadeIndex);
	if (texture(shadowSampler, vec3(shadowCoord.st, cascadeIndex)).r > shadowCoord.z)
	{
		visibility = 0.5f;
	}
    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility + pointLightIntensity * LIGHT_COLOR;
    outColor = texture(texSampler[nonuniformEXT(texIndex)], texCoord) * vec4(finalLight, 1.0);
}