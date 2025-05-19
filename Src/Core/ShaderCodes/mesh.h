struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

bool isSphereInsideFrustum(in vec3 center, in float radius, in vec4 frustumPlanes[6])
{
    for (int i = 0; i < 6; ++i)
    {
        vec4 plane = frustumPlanes[i];
        float d = dot(plane.xyz, center) + plane.w + radius;
        if (d < 0)
            return false;
    }
    return true;
}

bool isMeshletFacingCamera(in vec3 cameraPosition, in vec3 coneApex, in vec3 coneAxis, in float cutoff)
{
    return dot(normalize(coneApex - cameraPosition), coneAxis) <= cutoff;
}
