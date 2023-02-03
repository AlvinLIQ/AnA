#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat2 transform;
    uint sType;
    vec2 offset;
    vec2 resolution;
    vec3 color;
} push;

float rect(vec2 uv, float l, float t, float r, float b)
{
    float c = 0.;
    if (l <= uv.x && r >= uv.x && t <= uv.y && b >= uv.y)
        c = 1.;
    return c;
}

float rect2(vec2 uv, vec2 offset, vec2 size)
{
    float hw = size.x / 2., hh = size.y / 2.;
    return rect(uv, offset.x - hw, offset.y - hh, offset.x + hw, offset.y + hh);
}

float circle(vec2 uv, vec2 center, float radius)
{
    float c;
    vec2 t = center - uv;
    if (t.x * t.x + t.y * t.y <= radius * radius)
        c = 1.;
    else
        c = .0;

    return c;
}

float rounded_rect(vec2 uv, float l, float t, float r, float b, float radius)
{
    float c;
    c = rect(uv, l, t, r, b) - rect(uv, l, t, l + radius, t + radius)
        - rect(uv, l, b - radius, l + radius, b) - rect(uv, r - radius, t, r, t + radius)
        - rect(uv, r - radius, b - radius, r, b);
    
    c += circle(uv, vec2(l + radius, t + radius), radius) + circle(uv, vec2(r - radius, t + radius), radius)
        + circle(uv, vec2(l + radius, b - radius), radius) + circle(uv, vec2(r - radius, b - radius), radius);
    if (c >= 1.)
        c = 1.;
    return c;
}

float rounded_rect2(vec2 uv, vec2 offset, vec2 size, float radius)
{
    float hw = size.x / 2., hh = size.y / 2.;
    return rounded_rect(uv, offset.x - hw, offset.y - hh, offset.x + hw, offset.y + hh, radius);
}

void main() {
    vec2 uv = gl_FragCoord.xy / push.resolution;
    float ratio = push.resolution.y/push.resolution.x;
    uv.x /= ratio;
    float c;
    switch (push.sType)
    {
    case 1:
        c = 1.;
        break;
    case 2:
        c = circle(uv, push.offset + vec2(.5, .5), (push.resolution.y > push.resolution.x ? push.transform[0].x / ratio : push.transform[1].y) / 2);
        break;
    case 3:
        c = rect2(uv, vec2(push.offset.x / ratio, push.offset.y) + vec2(.5, .5), vec2(push.transform[0].x / ratio, push.transform[1].y));
        break;
    }
    
    vec3 color = c * push.color; 
    outColor = vec4(color, 1.0);
}