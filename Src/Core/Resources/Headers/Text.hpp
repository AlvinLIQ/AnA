#pragma once
#include "Renderable.hpp"
#include "../../Headers/Buffer.hpp"
#include <unordered_map>
#include <string>
#include <mutex>

namespace AnA
{
    struct CharacterInfo
    {
        uint32_t ch;
        uint32_t index;
    };
    struct TextData
    {
        float size;
        glm::vec2 offset;
        glm::vec3 color;
        glm::vec4 scissor;
        uint32_t chOffset;
        uint32_t count;
    };
    struct TextInfo
    {
        float size;
        glm::vec2 offset;
        glm::vec3 color;
        glm::vec4 scissor;
        std::string text;
        bool visible;
        size_t length;
    };
    struct TextMapData
    {
        TextInfo textInfo;
        uint32_t index;
        uint32_t capacity;
    };
    class Text : public Renderable
    {
    public:
        Text(Device* mDevice);
        ~Text();
        void Init();
        void Bind(CommandBuffer& commandBuffer, Shader& shader) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void Update() override;
        bool NeedUpdate() override;
        uint32_t Insert(const TextInfo& textInfo, uint32_t capacity = 0);
        void Remove(uint32_t id);
        void UpdateLayout(uint32_t id);
        void UpdateText(uint32_t id, const std::string& text);
        void ResetLayout();
        TextInfo* GetInfoById(uint32_t id);
        uint32_t GetTextCount();
    private:
        Device* aDevice{nullptr};
        size_t totalCharCount = 0;
        std::unordered_map<uint32_t, TextMapData> textMap{};
        std::unordered_map<int, int> characterMap{};
        std::vector<int> meshlets;
        std::mutex _mutex;
        void updateTextInfo(TextInfo& textInfo, uint32_t& chIndex, uint32_t& index, CharacterInfo* chInfoBuffer);
        void updateMeshlets(size_t meshletOffset);
        void updateAll();
        Buffer vertexBuffer;
        Buffer textBuffers[MAX_FRAMES_IN_FLIGHT];
        Buffer charInfoBuffers[MAX_FRAMES_IN_FLIGHT];
        uint32_t currentBufferIndex = 0;
        uint32_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
        Buffer drawCommandBuffer;
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;
        Buffer meshletBuffer;
        Buffer meshletIndexBuffer;
        Buffer countBuffers[MAX_FRAMES_IN_FLIGHT];
        bool needUpdate = false;
    };
}
