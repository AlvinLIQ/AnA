#pragma once

#include <glm/glm.hpp>
#include "Model.hpp"
#include "Descriptor.hpp"
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
        uint32_t textureId{};
    };
    struct Meshlet
    {
        uint32_t vertices[128];
        uint32_t indices[256 * 3];
        uint32_t indexCount;
        uint32_t vertexCount;
    };

    struct MeshInfo
    {
        char filePath[256];
        AnA::Transform transform;
        uint32_t tetureId{};
    };
    class Meshes
    {
    public:
        Meshes(Device* mDevice);
        ~Meshes();
        void Append(const std::vector<MeshInfo>& meshInfos);
        void Append(const MeshInfo* meshInfos, size_t count);
        void Append(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, Transform transform = {});
        void RemoveAt(uint32_t meshIndex);
        void RemoveAt(Range removeRange);
        void RemoveAt(std::vector<uint32_t> meshIndices);
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout, size_t offset, size_t size);
        void DrawIndirect(VkCommandBuffer commandBuffer);
        void DrawIndirect(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
        void DrawMesh(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void DrawMesh(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
        void DrawMeshIndirect(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
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

        Mesh& At(size_t index)
        {
            return meshes[index];
        }
        const Mesh* Get() const
        {
            return meshes.data();
        }
        size_t GetMeshCount() const
        {
            return meshes.size();
        }
        uint32_t GetBatchCount() const
        {
            uint32_t batchCount = meshes.size() / batchSize;
            if (meshes.size() % batchCount)
                batchCount++;

            return batchCount;
        }
        Descriptor* GetVertexDescriptor()
        {
            return vertexDescriptor;
        }
        bool EnableUpdate = false;
    private:
        Device* aDevice;
        Buffer vertexBuffer{};
        size_t vertexCount = 0;
        Buffer indexBuffer{};
        size_t indexCount = 0;
        Buffer drawIndexedIndirectBuffer{};
        Buffer drawMeshIndirectBuffer{};
        Buffer countBuffer{};
        std::vector<Mesh> meshes;
        uint32_t batchSize;
        std::vector<Range> updateQueue{};
        void commitVertexBufferUpdate(Model::Vertex* vertices, Model::Index* indices, Range& updateRange);
        uint32_t maxUpdateRange = 0;
        bool commandBufferNeedUpdate = false;
        std::unordered_map<uint32_t, uint32_t> textureIdMap{};
        std::vector<VkDescriptorImageInfo> textureInfos;
        std::vector<Descriptor*> samplersDescriptors;
        Descriptor* vertexDescriptor, *meshDescriptor;
        void createSSBODescriptor();
        void updateSSBODescriptor();
        void appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos);
        std::vector<Meshlet> meshlets;
        Buffer meshletsBuffer{};
        void buildMeshlets(
            uint32_t maxVerticesPerMeshlet = 128,  // max number of vertices per meshlet
            uint32_t maxIndicesPerMeshlet = 256 * 3);  // max number of indices per meshlet (i.e., triangles)
    };
}