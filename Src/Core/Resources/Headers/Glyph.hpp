#pragma once
#include "../../Headers/Buffer.hpp"
#include "Descriptor.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace AnA
{
    struct GlyphletInfo
    {
        uint32_t vertexCount;
        uint32_t indexCount;
        glm::vec2 position;
    };
    struct Glyphlet
    {
        std::vector<uint32_t> vertices;
        std::vector<uint8_t> indices;
    };
    struct GlyphInfo
    {
        String text;
        glm::vec3 color;
        float scale;
    };
    class Glyphs
    {
    public:
        Glyphs(Device* mDevice);
        ~Glyphs();
        void Append(const GlyphInfo& info, uint32_t& id);
        void Update(uint32_t id);
        void DrawMesh(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
    private:
        Device* aDevice;

        std::unordered_map<uint32_t, GlyphInfo> glyphMap{};
        Descriptor* glyphletDescriptor{};

        std::vector<GlyphletInfo> glyphletInfos{};
        std::vector<Buffer> glyphletInfoBuffers{};
        uint8_t currentBufferIndex = 0;
        uint8_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
        std::vector<Glyphlet> glyphlets;
        Buffer glyphletBuffer{};
        void buildGlyphlets();
    };
}