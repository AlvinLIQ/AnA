#version 450
#extension GL_EXT_scalar_block_layout: enable

layout(push_constant) uniform PushConsts {
    vec2 resolution;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in vec4 bounding;

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy;
    if (uv.x < bounding.x || uv.x > bounding.z ||
            uv.y < bounding.y || uv.y > bounding.w)
        discard;

    outColor = vec4(fragColor, 1.0);
}
