#version 460

#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4
#define ANA_TEXT 5
#define ANA_SPHERE 6

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out uint outObjectID;

layout(push_constant) uniform Push {
    uint sType;
    vec3 color;
} push;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 proj;
    mat4 view;
    vec2 resolution;
} cbo;

struct Object{
    mat4 model;
};

layout(std140, set = 1, binding = 0) buffer ObjectBuffer {
    Object objects[];
} objectBuffer;

struct Ray{
    vec3 center;
    vec3 direction;
};

const vec3 LIGHT_DIRECTION = normalize(vec3(1., -3., 1.));

void main() {
    //gl_Position = vec4(push.transform * position + push.offset * 2, 0.0, 1.0);
    if (push.sType == ANA_MODEL)
    {
        gl_Position = cbo.proj * cbo.view * objectBuffer.objects[gl_BaseInstance].model * vec4(position, 1.0);
        vec3 normalWorldSpace = normalize(mat3(objectBuffer.objects[gl_BaseInstance].model) * normal);

        float lightIntensity = max(dot(normalWorldSpace, LIGHT_DIRECTION), 0);
        fragColor = lightIntensity * vec3(1.0) + 0.033;
    }
    else
    {
        gl_Position = objectBuffer.objects[gl_BaseInstance].model * vec4(position, 1.0);
        fragColor = push.color;
    }
    outTexCoord = uv;
    outObjectID = gl_BaseInstance;
    //gl_Position = push.projectionMatrix * push.transformMatrix * vec4(position, 1.0);
}