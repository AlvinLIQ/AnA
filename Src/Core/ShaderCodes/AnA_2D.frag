#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

float rect(vec2 uv, float l, float t, float r, float b)
{
    float c = 0.;
    if (l <= uv.x && r >= uv.x && t <= uv.y && b >= uv.y)
        c = 1.;
    return c;
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

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800, 600);
    uv.x /= 3./4.;
    
    float c;
    c = rounded_rect(uv, 0.4, 0.4, 0.6, 0.6, 0.03);
    vec3 color = c * push.color; 
    outColor = vec4(color, 1.0);
}