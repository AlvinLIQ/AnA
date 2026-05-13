#version 450
layout(location = 0) in vec4 vertexPos;
layout(location = 0) out vec4 outColor;

const mat3 mat = mat3(0.820425, -0.482072, 0.307423,
        0.000000, 0.537684, 0.843147,
        -0.571754, -0.691739, 0.441129);

void main()
{
    //const float maxY = 6.468271;
    //const float minY = 2.403305;
    const float maxY = 0.0;
    const float minY = -5.0f;
    vec3 color;

    if (vertexPos.x == 0. && vertexPos.y == 0. && vertexPos.z == 0.)
        color = vec3(0.0, 1.0, 0.0);
    else
        color = vec3(1.0, 0.0, 0.0);
    outColor = vec4(color, 1.);
}
