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
		float dist = texture(shadowSampler, vec3(shadowCoord.xy + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 0.5;
		}
	}
	return shadow;

}

void main()
{
    float pointLightIntensity = max(dot(normalSpace, normalize(LIGHT_DIRECTION - vertex)), 0);
    float diffuseLightItensity = max(dot(normalSpace, normalize(lbo.direction)), 0);
	
    uint cascadeIndex = 0;
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(viewPos.z < ubo.cascades[i].split) {	
			cascadeIndex = i + 1;
		}
	}
    vec4 shadowCoord = biasMat * ubo.cascades[0].viewProj * vec4(vertex, 1.0);
    float visibility = textureProj(shadowCoord, vec2(0.), cascadeIndex);

    vec3 finalLight = (diffuseLightItensity * lbo.color + lbo.ambient) * visibility + pointLightIntensity * LIGHT_COLOR;
	outColor = texture(texSampler[nonuniformEXT(texIndex)], texCoord) * vec4(finalLight, 1.0);
    //outColor = texture(shadowSampler, vec3(gl_FragCoord.xy / cbo.resolution, 2)).r * vec4(1.);
}