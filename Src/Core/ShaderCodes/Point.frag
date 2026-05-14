#version 450
layout(location = 0) in vec4 vertexPos;
layout(location = 1) flat in uint vertexIndex;
layout(location = 2) flat in float intensity;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) buffer CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec4 position;
    vec2 resolution;
    vec2 cursorPosition;
    uint selectedIndex;
} cbo;

const mat3 mat = mat3(0.820425, -0.482072, 0.307423,
        0.000000, 0.537684, 0.843147,
        -0.571754, -0.691739, 0.441129);

void main()
{
    //const float maxY = 6.468271;
    //const float minY = 2.403305;
    const float maxY = 0.0;
    const float minY = -5.0f;
    vec2 cursorPos = cbo.cursorPosition;
    vec3 color;
    if (distance(cursorPos, gl_FragCoord.xy) < 4)
    {
        cbo.selectedIndex = vertexIndex;
    }
    outColor = vec4(intensity, 0.0, 0.0, intensity);
}
