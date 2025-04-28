#version 460
#extension GL_EXT_nonuniform_qualifier : enable 

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint texIndex;
layout(location = 2) in vec3 normalSpace;
layout(location = 3) in vec3 vertex;
layout(location = 4) in vec4 viewPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform CameraBufferObject {
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

#define SHADOW_MAP_CASCADE_COUNT 3
struct Cascade {
    mat4 viewProj;
    float split;
};
layout (set = 5, binding = 0) uniform UBO {
	Cascade[SHADOW_MAP_CASCADE_COUNT] cascades;
} ubo;

const vec3 LIGHT_POS = vec3(4., -2., 1.);
const vec3 LIGHT_COLOR = vec3(0.6, 0.7, 0.5);

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float shadow = 1.0;
	float bias = 0.003;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(shadowSampler, vec3(shadowCoord.xy + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 1.0 - (0.5 * smoothstep(21.0, 20.8, viewPos.z));
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
    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_POS - vertex)), 0);
    float diffuseLightItensity = ((dot(normalSpace, normalize(lbo.direction))) + 2.0) * 0.5;
	
    uint cascadeIndex = SHADOW_MAP_CASCADE_COUNT - 1;
	for(uint i = 1; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		if(viewPos.z + 8.5 < ubo.cascades[i].split) {	
			cascadeIndex = i - 1;
			break;
		}
	}
    vec4 shadowCoord = biasMat * ubo.cascades[cascadeIndex].viewProj * vec4(vertex, 1.0);
    float visibility = filterPCF(shadowCoord, cascadeIndex);
	// when z > 28, shadow start to fade out
	// f(1) = 0
	// f(x) = 1

    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility + pointLightIntensity * LIGHT_COLOR;
	outColor = texture(texSampler[nonuniformEXT(texIndex)], texCoord) * vec4(finalLight, 1.0);
    //outColor = texture(shadowSampler, vec3(gl_FragCoord.xy / cbo.resolution, 2)).r * vec4(1.);
}