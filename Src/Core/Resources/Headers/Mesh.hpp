#pragma once

#include <glm/glm.hpp>
#include "Renderable.hpp"
#include "Model.hpp"
#include "Descriptor.hpp"
#include "CommandBuffer.hpp"
#include "../../Headers/Types.hpp"

namespace AnA
{
    struct Mesh
    {
        AnA::Transform transform;
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t textureId{};
    };
    struct Meshlet
    {
        uint32_t vertices[128];
        uint8_t indices[256 * 3];
        uint32_t indexCount;
        uint32_t vertexCount;
        glm::vec3 center;
        glm::vec3 normal;
        float cutoff;
        float radius;
    };

    struct MeshInfo
    {
        char filePath[256];
        AnA::Transform transform;
        uint32_t tetureId{};
    };

    struct FrustumPlanes
    {
        glm::vec4 planes[6];
        static void ExtractFrustumPlanes(const glm::mat4& m, FrustumPlanes& fp);
    };

    struct BoundingSphere
    {
        glm::vec3 center;
        float radius;
        glm::vec3 normal;
        float cutoff;
    };

    class Meshes : public Renderable
    {
    public:
        Meshes(Device* mDevice);
        virtual ~Meshes();
        void Append(const std::vector<MeshInfo>& meshInfos);
        void Append(const MeshInfo* meshInfos, size_t count);
        void Append(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, Transform transform = {});
        void RemoveAt(uint32_t meshIndex);
        void RemoveAt(Range removeRange);
        void RemoveAt(std::vector<uint32_t> meshIndices);
        void Bind(CommandBuffer& commandBuffer);
        void Bind(CommandBuffer& commandBuffer, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
        void DrawMesh(CommandBuffer& commandBuffer);
        void DrawMesh(CommandBuffer& commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
        void DrawMeshIndirect(CommandBuffer& commandBuffer);
        void DrawMeshIndirect(CommandBuffer& commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout);
        bool NeedUpdate() override
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
        void Update() override;
        void UpdateBuffers(Range updateRange);
        void UpdateMeshlets();
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
            uint32_t batchCount = static_cast<uint32_t>(meshes.size()) / batchSize;
            if (meshes.size() % batchCount)
                batchCount++;

            return batchCount;
        }
        const Descriptor* GetVertexDescriptor() const
        {
            return vertexDescriptor;
        }
        const Descriptor* GetMeshDescriptor() const
        {
            return meshDescriptor;
        }
        VkDescriptorSet GetVertexDescriptorSet() const
        {
            return vertexDescriptor->GetSets()[currentBufferIndex];
        }
        VkDescriptorSet GetMeshDescriptorSet() const
        {
            return meshDescriptor->GetSets()[currentBufferIndex];
        }
        VkDescriptorSet GetSamplersDescriptorSet() const
        {
            return samplersDescriptors[0]->GetSets()[0];
        }
        bool EnableUpdate = false;
    private:
        Device* aDevice;
        std::vector<Model::Vertex> vertices{};
        std::vector<Buffer> vertexBuffers{};
        size_t vertexCount = 0;
        std::vector<Model::Index> indices{};
        std::vector<Buffer> indexBuffers{};
        size_t indexCount = 0;
        Buffer drawIndexedIndirectBuffer{};
        Buffer drawMeshIndirectBuffer{};
        Buffer countBuffer{};
        std::vector<Mesh> meshes;
        uint32_t batchSize;
        std::vector<Range> updateQueue{};
        void applyVertexBufferUpdate(Model::Vertex* vertices, Model::Index* indices, Range& updateRange);
        uint32_t maxUpdateRange = 0;
        bool commandBufferNeedUpdate = false;
        std::unordered_map<uint32_t, uint32_t> textureIdMap{};
        std::vector<VkDescriptorImageInfo> textureInfos;
        std::vector<Descriptor*> samplersDescriptors;
        Descriptor* vertexDescriptor{nullptr}, *meshDescriptor{nullptr};
        void createSSBODescriptor();
        void updateSSBODescriptor();
        void appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos);
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;
        std::vector<Meshlet> meshlets;
        std::vector<Buffer> meshletBuffers{};
        std::vector<Buffer> meshletCullingBuffers{};
        uint32_t numOfGroup = 64;
        void buildMeshlets();
        void buildMeshletsWithOptimizer();
        uint8_t currentBufferIndex = 0;
        uint8_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
    };
}