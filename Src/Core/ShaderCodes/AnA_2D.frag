#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

float rect(vec2 uv, float l, float t, float r, float b, float blur)
{
    float c;
    c = smoothstep(uv.x, uv.x - blur, l) 
        - smoothstep(uv.x, uv.x - blur, r)
        - smoothstep(t, t - blur, uv.y)
        - smoothstep(uv.y, uv.y - blur, b);
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

float rounded_rect(vec2 uv, float l, float t, float r, float b, float blur, float radius)
{
    float c;
    c = rect(uv, l, t, r, b, blur) - rect(uv, l, t, l + radius, t + radius, blur)
    - rect(uv, r - radius, t, r, t + radius, blur) - rect(uv, l, b - radius, l + radius, b, blur)
    - rect(uv, r - radius, b - radius, r, b, blur);
    c += circle(uv, vec2(l + radius, t + radius), radius) + circle(uv, vec2(r - radius, t + radius), radius)
    + circle(uv, vec2(l + radius, b - radius), radius) + circle(uv, vec2(r - radius, b - radius), radius);
    return c;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800, 600);
    uv.x /= 3./4.;
    
    float c;
    c = rect(uv, 0.4, 0.4, 0.6, 0.6, 0.005);
    vec3 color = c * push.color; 
    outColor = vec4(color, 1.0);
}