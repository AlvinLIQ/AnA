#version 450

#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4
#define ANA_TEXT 5

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    uint sType;
    uint index;
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

layout(std140, set = 1, binding = 1) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

layout(set = 2, binding = 2) uniform sampler2D texSampler;

float rect(vec2 uv, float l, float t, float r, float b)
{
    float c = 0.;
    if (l <= uv.x && r >= uv.x && t <= uv.y && b >= uv.y)
        c = 1.;
    return c;
}

float rect2(vec2 uv, vec2 offset, vec2 size)
{
    return rect(uv, offset.x, offset.y, offset.x + size.x, offset.y + size.y);
}

float ellipse(vec2 uv, vec2 center, vec2 radius)
{
    float c;
    vec2 t = center - uv;
    if (t.x * t.x / (radius.x * radius.x) + t.y * t.y / (radius.y * radius.y) <= 1)
        c = 1.;
    else
        c = .0;

    return c;
}

float rounded_rect(vec2 uv, float l, float t, float r, float b, vec2 radius)
{
    float c;
    c = rect(uv, l + radius.x, t, r - radius.x, b) + rect(uv, l, t + radius.y, r, b - radius.y);
    
    c += ellipse(uv, vec2(l + radius.x, t + radius.y), radius) + ellipse(uv, vec2(r - radius.x, t + radius.y), radius)
        + ellipse(uv, vec2(l + radius.x, b - radius.y), radius) + ellipse(uv, vec2(r - radius.x, b - radius.y), radius);
    if (c >= 1.)
        c = 1.;
    return c;
}

float rounded_rect2(vec2 uv, vec2 offset, vec2 size, vec2 radius)
{
    return rounded_rect(uv, offset.x, offset.y, offset.x + size.x, offset.y + size.y, radius);
}

void main() {
    if (push.sType == ANA_MODEL || push.sType == ANA_TEXT)
    {
        outColor = texture(texSampler, texCoord) * vec4(fragColor, 1.0);
        return;
    }
    
    uint index = push.index;
    vec2 uv = gl_FragCoord.xy / cbo.resolution;
    //float ratio = cbo.resolution.y/cbo.resolution.x;
    //uv.x /= ratio;
    float c = 0.;
    vec2 offset = vec2((objectBuffer.objects[index].model[3].x + (1. - objectBuffer.objects[index].model[0].x)) / 2., (objectBuffer.objects[index].model[3].y + (1. - objectBuffer.objects[index].model[1].y)) / 2.);
    switch (push.sType)
    {
    case ANA_RECTANGLE:
        c = rect2(uv, offset, vec2(objectBuffer.objects[index].model[0].x, objectBuffer.objects[index].model[1].y));
        break;
    case ANA_ELLIPSE:
        c = ellipse(uv, vec2(offset.x + objectBuffer.objects[index].model[0].x / 2., offset.y + objectBuffer.objects[index].model[1].y / 2.), vec2(objectBuffer.objects[index].model[0].x / 2, objectBuffer.objects[index].model[1].y / 2));
        break;
    case ANA_CURVED_RECTANGLE:
        c = rounded_rect2(uv, offset, vec2(objectBuffer.objects[index].model[0].x, objectBuffer.objects[index].model[1].y), vec2(0.03));
        break;
    }

    outColor = vec4(c * texture(texSampler, texCoord));
}