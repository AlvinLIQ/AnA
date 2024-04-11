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
        uint32_t textureId{(uint32_t)-1};
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
        void RemoveAt(uint32_t meshIndex);
        void RemoveAt(Range removeRange);
        void RemoveAt(std::vector<uint32_t> meshIndices);
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
        bool NeedUpdate()
        {
            return updateQueue.size();
        }
        bool BeginCommandBufferUpdate()
        {
            return commandBufferNeedUpdate;
        }
        void EndCommandBufferUpdate()
        {
            commandBufferNeedUpdate = false;
        }
        void CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer);
        void CommitBufferUpdate();
        void UpdateAll();
        void UpdateBuffers(Range updateRange);
        void UpdateVertexPositions(Mesh& mesh);
        void UpdateVertexPositions(Range updateRange);

        Mesh& GetAt(size_t index)
        {
            return meshes[index];
        }
        size_t GetMeshCount() const
        {
            return meshes.size();
        }
    private:
        Device& aDevice;
        Buffer* vertexBuffer{nullptr};
        size_t vertexCount = 0;
        Buffer* indexBuffer{nullptr};
        size_t indexCount = 0;
        std::vector<Mesh> meshes;
        std::vector<Range> updateQueue{};
        uint32_t maxUpdateRange = 0;
        bool commandBufferNeedUpdate = false;
    };
}