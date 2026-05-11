#version 450
layout(location = 0) in vec4 vertexPos;
layout(location = 0) out vec4 outColor;

const mat3 mat = mat3
    (0.820425, -0.482072, 0.307423,
0.000000, 0.537684, 0.843147,
-0.571754, -0.691739, 0.441129);

void main()
{
    //const float maxY = 6.468271;
    //const float minY = 2.403305;
    const float maxY = 1.462684;
    const float minY = 0.394702;
    float y = ((mat * vertexPos.xyz).z - minY) * 3. / (maxY - minY);
    vec3 color = vec3(0.);
    if (y <= 1.)
    {
        color.r = 1. - y;
        color.g = y;
    }
    else if (y <= 2.)
    {
        color.g = 2. - y;
        color.b = y - 1.;
    }
    else if (y <= 3.)
    {
        color.b = 3. - y;
    }
    outColor = vec4(color, 1.);
}