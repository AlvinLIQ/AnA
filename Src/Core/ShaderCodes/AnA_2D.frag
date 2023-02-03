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
    return rect(uv, offset.x, offset.y, offset.x + size.x, offset.y + size.y);
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
    return rounded_rect(uv, offset.x, offset.y, offset.x + size.x, offset.y + size.y, radius);
}

void main() {
    vec2 uv = gl_FragCoord.xy / push.resolution;
    float ratio = push.resolution.y/push.resolution.x;
    //uv.x /= ratio;
    float c = 0.;
    switch (push.sType)
    {
    case 1:
        c = rect2(uv, vec2(push.transform[0].x / 2. + .5, .5 - push.transform[1].y / 2.), vec2(push.transform[0].x, push.transform[1].y));
        break;
    case 2:
        c = circle(uv, push.offset + vec2(.5, .5), (push.resolution.y > push.resolution.x ? push.transform[0].x / ratio : push.transform[1].y) / 2);
        break;
    case 3:
        c = rounded_rect2(uv, vec2(push.transform[0].x / 2. + .5, .5 - push.transform[1].y / 2.), vec2(push.transform[0].x, push.transform[1].y), 0.03);
        break;
    }
    
    vec3 color = c * push.color; 
    outColor = vec4(color, 1.0);
}