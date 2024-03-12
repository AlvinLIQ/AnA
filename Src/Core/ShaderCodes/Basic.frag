#version 460

#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4
#define ANA_TEXT 5
#define ANA_SPHERE 6

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in uint objectIndex;

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

struct Ray{
    vec3 center;
    vec3 direction;
}

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

float circle(vec2 uv, vec2 center, float radius)
{
    float c;
    float d = length(center - uv);
    c = smoothstep(radius, radius - 0.006, d);

    return c;
}

vec4 sphere(vec2 uv, vec2 center, float radius, vec3 lightDirection, vec3 sColor)
{
    float c = circle(uv, center, radius);

    vec2 normalXY = uv - center;
    vec3 normal = normalize(vec3(normalXY, sqrt(radius * radius - length(normalXY) * length(normalXY))));
    return vec4(c * 0.4 * (dot(normal, lightDirection) + 1.0) * sColor, c);
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
    if (push.sType == ANA_MODEL)
    {
        outColor = texture(texSampler, texCoord) * vec4(fragColor, 1.0);
        return;
    }
    float aspect = cbo.resolution.x / cbo.resolution.y;
    vec2 uv = gl_FragCoord.xy / cbo.resolution;
    uv.x *= aspect;
    float c = 0.;
    vec4 color = texture(texSampler, texCoord);
    if (color.a == 0.)
            discard;
    vec2 offset = vec2((objectBuffer.objects[objectIndex].model[3].x + (1. - objectBuffer.objects[objectIndex].model[0].x)) / 2., (objectBuffer.objects[objectIndex].model[3].y + (1. - objectBuffer.objects[objectIndex].model[1].y)) / 2.);
    offset.x *= aspect;
    switch (push.sType)
    {
    case ANA_RECTANGLE:
        c = rect2(uv, offset, vec2(objectBuffer.objects[objectIndex].model[0].x, objectBuffer.objects[objectIndex].model[1].y));
        break;
    case ANA_SPHERE:
        vec2 center = vec2(offset.x + objectBuffer.objects[objectIndex].model[0].x / 2., offset.y + objectBuffer.objects[objectIndex].model[1].y / 2.);
        vec2 radius = vec2(objectBuffer.objects[objectIndex].model[0].x / 2, objectBuffer.objects[objectIndex].model[1].y / 2);
        outColor = sphere(uv, center, radius.x, normalize(mat3(cbo.view) * vec3(0.5, -3.0, 0.4)), fragColor);
        return;
    case ANA_CURVED_RECTANGLE:
        c = rounded_rect2(uv, offset, vec2(objectBuffer.objects[objectIndex].model[0].x, objectBuffer.objects[objectIndex].model[1].y), vec2(0.03));
        break;
    default:
        c = 1.;
        break;
    }

    outColor = vec4(c * color * vec4(fragColor, 1.0));
}