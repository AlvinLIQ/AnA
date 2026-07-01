#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec4 baseColor;
layout(location = 1) in vec4 bounding;
layout(location = 2) in vec2 texCoord;
layout(location = 3) flat in uint texIndex;
layout(location = 4) flat in uint texLayer;
layout(location = 0) out vec4 outColor;

//layout(set = 0, binding = 0) uniform sampler2DArray texSampler[];

layout(push_constant) uniform PushConstants
{
    uint64_t shapePtr;
    vec2 resolution;
};

void main()
{
    //outColor = vec4(baseColor, 1.0);
    vec2 uv = gl_FragCoord.xy / resolution;
    if (uv.x < bounding.x || uv.x > bounding.z ||
            uv.y < bounding.y || uv.y > bounding.w)
        discard;

    vec4 texColor = vec4(1.0);//texture(texSampler[nonuniformEXT(texIndex)], vec3(texCoord, texLayer));
    if (texColor.a < 0.5 || baseColor.w == 0.)
        discard;
    outColor = vec4(baseColor.x * texColor.x, baseColor.y * texColor.y, baseColor.z * texColor.z, baseColor.w);
}
