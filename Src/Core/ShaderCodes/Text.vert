#version 450

layout(location = 0) out vec3 fragColor;

layout(std430, set = 0, binding = 0) buffer CharacterVertexSSBO
{
    vec2 vertices[];
};

struct Character
{
    vec2 baseSize;
};

layout(std430, set = 0, binding = 1) buffer CharacterSSBO
{
    Character characters[];
};

struct TextInfo
{
    uint lineWidth;
    float spacing;
    float scale;
    vec2 offset;
    vec3 color;
};

layout(std430, set = 1, binding = 0) buffer CharacterTranformSSBO
{
    TextInfo textInfos[];
};

layout(std430, set = 1, binding = 1) buffer CharacterIndexSSBO
{
    uint textIndexCount;
    uint textIndices[];
};

void main()
{
    uint tid = textIndices[gl_VertexIndex];
    uint cid = textIndices[gl_VertexIndex + textIndexCount];

    uint lineWidth = textInfos[tid].lineWidth;
    float spacing = textInfos[tid].spacing;
    vec2 cOffset = vec2(float(cid % lineWidth) * (1.0 + spacing), float(cid / lineWidth) * spacing) + textInfos[tid].offset;
    gl_Position = vec4(textInfos[tid].scale * (vertices[gl_VertexIndex] + cOffset), 0.0, 1.0);
    fragColor = textInfos[tid].color;
}