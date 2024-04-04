#pragma once

#include <glm/glm.hpp>
#include "Model.hpp"
#include "../../Headers/Types.hpp"

namespace AnA
{
    struct Mesh
    {
        AnA::Transform transform;
        std::vector<Model::Vertex> vertices;
        std::vector<Model::Index> indices;
        uint32_t vertexOffset;
        uint32_t indexOffset;
    };
    struct MeshInfo
    {
        std::string filePath;
        AnA::Transform transform;
    };
    class Meshes
    {
    public:
        Meshes(Device& mDevice);
        ~Meshes();
        void Append(const std::vector<MeshInfo>& meshInfos);
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
        bool NeedUpdate()
        {
            return updateQueue.size();
        }
        bool BeginCommandBufferUpdate()
        {
            return outdatedCommandBufferCount > 0;
        }
        void EndCommandBufferUpdate()
        {
            if (outdatedCommandBufferCount > 0)
                --outdatedCommandBufferCount;
        }
        void CommitBufferUpdate();
        void UpdateBuffers(Range updateRange);
        void UpdateVertexPositions(Mesh& mesh);
        void UpdateVertexPositions(Range updateRange);

        uint32_t GetOutdatedCommandBufferCount() const
        {
            return outdatedCommandBufferCount;
        }
        Texture* TestTexture{nullptr};
    private:
        Device& aDevice;
        Buffer* vertexBuffer;
        size_t vertexCount = 0;
        Buffer* indexBuffer;
        size_t indexCount = 0;
        std::vector<Mesh> meshes;
        std::vector<Range> updateQueue{};
        uint32_t maxUpdateRange = 0;
        uint32_t outdatedCommandBufferCount = MAX_FRAMES_IN_FLIGHT;
    };
}