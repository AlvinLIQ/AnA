#version 460

layout(location = 0) in vec3 baseColor;
layout(location = 0) out vec4 outColor;

float rect(vec2 uv, float l, float t, float r, float b)
{
    float c = 1.;
    c -= smoothstep(l, l - 0.001, uv.x) + smoothstep(uv.x, uv.x - 0.001, r)
        + smoothstep(t, t - 0.001, uv.y) + smoothstep(uv.y, uv.y - 0.001, b);
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

void main()
{
    outColor = vec4(baseColor, 1.0);
}
