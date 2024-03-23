#version 460

#define ANA_MODEL 1
#define ANA_TRIANGLE 2
#define ANA_RECTANGLE 4
#define ANA_ELLIPSE 8
#define ANA_CURVED_RECTANGLE 16
#define ANA_TEXT 32
#define ANA_SPHERE 64

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out uint outObjectID;
layout(location = 3) out vec3 outNormalSpace;
layout(location = 4) out vec3 outVertex;
layout(location = 5) out vec4 outShadowCoord;

layout(push_constant) uniform Push {
    uint sType;
    vec3 color;
} push;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

layout(set = 2, binding = 0) uniform LightBufferObject {
    mat4 proj;
    mat4 view;
    vec3 direction;
    vec3 color;
    float ambient;
} lbo;

struct Ray{
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));

const mat4 biasMat = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );

#define PI 3.14

mat4 transform(vec3 scale, vec3 rotation, vec3 transition)
{
    float c3 = cos(rotation.z);
    float s3 = sin(rotation.z);
    float c2 = cos(rotation.x);
    float s2 = sin(rotation.x);
    float c1 = cos(rotation.y);
    float s1 = sin(rotation.y);
    return mat4(
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
            vec4(transition, 1.0)
    );
}

void main() {
    //gl_Position = vec4(push.transform * position + push.offset * 2, 0.0, 1.0);
    if (push.sType == ANA_MODEL)
    {
        vec4 vertex = objectBuffer.objects[gl_BaseInstance].model * vec4(position, 1.0);
        gl_Position = cbo.proj * cbo.view * vertex;
        outNormalSpace = normalize(mat3(objectBuffer.objects[gl_BaseInstance].model) * normal);
        outVertex = vertex.xyz / vertex.w;
        mat4 dView = mat4(lbo.view);
        dView[3] = vec4(mat3(cbo.invView) * vec3(cbo.view[3]), 1.0);
        outShadowCoord = biasMat * lbo.proj * dView * vertex;
        fragColor = color;
    }
    else
    {
        mat4 model = objectBuffer.objects[gl_BaseInstance].model;
        //float aspect = cbo.resolution.x / cbo.resolution.y;
        //model[0] *= aspect;
        gl_Position = model * vec4(position, 1.0);
        fragColor = push.color;
    }
    outTexCoord = uv;
    outObjectID = gl_BaseInstance;
    //gl_Position = push.projectionMatrix * push.transformMatrix * vec4(position, 1.0);
}