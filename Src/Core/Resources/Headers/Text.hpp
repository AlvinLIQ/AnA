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
        uint32_t chOffset;
        uint32_t count;
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
        void Init();
        void Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void Update() override;
        bool NeedUpdate() override;
        uint32_t Insert(const TextInfo& textInfo);
    private:
        Device* aDevice{nullptr};
        size_t totalTextLen = 0;
        std::unordered_map<uint32_t, uint32_t> textMap{};
        std::unordered_map<char, char> characterMap{};
        std::vector<TextInfo> textInfos{};
        std::vector<char> meshlets;
        void updateMeshlets(size_t meshletOffset);
        Buffer vertexBuffer;
        Buffer textBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer charInfoBuffers[MAX_FRAMES_IN_FLIGHT];
        uint32_t currentBufferIndex = 0;
        uint32_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
        Buffer drawCommandBuffer;
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;
        Buffer meshletBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer meshletVertexBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer meshletIndexBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer countBuffers[MAX_FRAMES_IN_FLIGHT];
        uint32_t drawCount = 0;
        Descriptor* vertexDescriptor{};
        Descriptor* charInfoDescriptor{};
        Descriptor* meshDescriptor{};
        void createSSBODescriptor();
        void updateSSBODescriptor();
        bool needUpdate = false;
    };
}