#version 450
#extension GL_EXT_scalar_block_layout: enable

layout(push_constant) uniform PushConsts {
    vec2 resolution;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in vec4 scissor;

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5) / push.resolution;
    if (scissor.x > uv.x || scissor.y > uv.y || 1.0 - scissor.z < uv.x || 1.0 - scissor.w < uv.y)
        discard; //culled
    outColor = vec4(fragColor, 1.0);
}
