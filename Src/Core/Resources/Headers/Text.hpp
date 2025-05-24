#pragma once
#include "Renderable.hpp"
#include "Descriptor.hpp"
#include "../../Headers/Buffer.hpp"
#include <unordered_map>
#include <string>

namespace AnA
{
    struct CharacterInfo
    {
        char ch;
        uint32_t index;
    };
    struct TextData
    {
        glm::vec2 scale;
        glm::vec2 offset;
    };
    struct TextInfo
    {
        glm::vec2 scale;
        glm::vec2 offset;
        std::string text;
    };
    class Text : public Renderable
    {
    public:
        Text(Device* mDevice);
        ~Text();
        void Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void Update() override;
        bool NeedUpdate() override;
        uint32_t Insert(const TextInfo& textInfo);
        std::unordered_map<uint32_t, TextInfo> TextMap{};
    private:
        Device* aDevice{nullptr};
        size_t totalTextLen = 0;
        Buffer textBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer charInfoBuffers[MAX_FRAMES_IN_FLIGHT];
        Descriptor* charInfoDescriptor{nullptr};
        uint32_t currentBufferIndex = 0;
        uint32_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
        Buffer drawCommandBuffer;
        Buffer countBuffers[MAX_FRAMES_IN_FLIGHT];
        uint32_t drawCount = 0;
        bool needUpdate = false;
    };
}