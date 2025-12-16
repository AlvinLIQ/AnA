
#define MAX_VERTICES 64
#define MAX_PRIMITIVES 128
#define TASK_WORKGROUP 64

const uint bytesToBits[4] = {0u, 8u, 16u, 24u};
const uint bytesMask[4] = {0xFFFFFFu, 0xFFFFFFu, 0xFFFF00u, 0xFF0000u};
const uint rQBytesToBits[4] = {0u, 8u, 8u, 8u};
const uint rQBytesMask[4] = {0x0u, 0x0u, 0xFFu, 0xFFFFu};

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

struct Object
{
    vec3 center;
    float radius;
    mat4 transform;
};

struct Meshlet
{
    uint vertexOffset;
    uint indexOffset;
    uint vertexCount;
    uint indexCount;
};

struct MeshletID
{
    uint meshletID;
    uint objectID;
};

struct MeshPayload
{
    uint visibleCount;
    uint visibleIndices[TASK_WORKGROUP];
};

struct BoundingSphere
{
    vec3 center;
    float radius;
    vec3 normal;
    float cutoff;
    vec3 coneApex;
    float padding;
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
