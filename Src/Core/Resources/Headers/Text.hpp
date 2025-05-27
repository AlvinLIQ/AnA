#pragma once
#include "Renderable.hpp"
#include "Descriptor.hpp"
#include "../../Headers/Buffer.hpp"
#include <unordered_map>
#include <string>
#include <mutex>

namespace AnA
{
    struct CharacterInfo
    {
        char ch;
        uint32_t index;
    };
    struct TextData
    {
        float size;
        glm::vec2 offset;
        glm::vec3 color;
        uint32_t chOffset;
        uint32_t count;
    };
    struct TextInfo
    {
        float size;
        glm::vec2 offset;
        glm::vec3 color;
        std::string text;
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
        void Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void Update() override;
        bool NeedUpdate() override;
        uint32_t Insert(const TextInfo& textInfo, uint32_t capacity = 0);
        void Remove(uint32_t id);
        void UpdateLayout(uint32_t id);
        void UpdateText(uint32_t id, const std::string& text);
        TextInfo* GetInfoById(uint32_t id);
    private:
        Device* aDevice{nullptr};
        size_t totalCharCount = 0;
        std::unordered_map<uint32_t, TextMapData> textMap{};
        std::unordered_map<char, char> characterMap{};
        std::vector<char> meshlets;
        std::mutex _mutex;
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
        Buffer meshletVertexBuffer;
        Buffer meshletIndexBuffer;
        Buffer countBuffers[MAX_FRAMES_IN_FLIGHT];
        Descriptor* vertexDescriptor{};
        Descriptor* charInfoDescriptor{};
        Descriptor* meshDescriptor{};
        void createSSBODescriptor();
        void updateSSBODescriptor();
        bool needUpdate = false;
    };
}