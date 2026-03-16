#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include "Renderable.hpp"
#include "Model.hpp"
#include "Descriptor.hpp"
#include "CommandBuffer.hpp"
#include "../../Headers/Buffer.hpp"
#include "../../Headers/Types.hpp"

namespace AnA
{
    struct Mesh
    {
        AnA::Transform transform;
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t textureId{};
        uint32_t modelID;
    };
    struct MeshletInfo
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t vertexCount;
        uint32_t indexCount;
    };

    struct Object
    {
        glm::vec3 center;
        float radius;
        glm::vec4 halfVolume;
        glm::mat4 transform;
    };

    struct MeshletID
    {
        uint32_t meshletID;
        uint32_t objectID;
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
        glm::vec3 coneApex;
        float padding;
    };

    struct CollisionData
    {
        glm::uvec2 pair;
        glm::vec3 normal;
        float penetration;
    };

    class Scene : public Renderable
    {
    public:
        struct SceneFrameResources
        {
            Buffer objectBuffer;
            Buffer meshletIDBuffer;
        };
        Scene(Device* mDevice);
        virtual ~Scene();
        void Init();
        void Append(const std::vector<MeshInfo>& meshInfos);
        void Append(const MeshInfo* meshInfos, size_t count);
        void Append(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, Transform transform = {}, uint32_t textureId = 0);
        void RemoveAt(uint32_t meshIndex);
        void RemoveAt(Range removeRange);
        void RemoveAt(std::vector<uint32_t> meshIndices);
        void Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        bool NeedUpdate() override
        {
            return needUpdate;
        }
        bool BeginCommandBufferUpdate()
        {
            return commandBufferNeedUpdate;
        }
        void EndCommandBufferUpdate()
        {
            commandBufferNeedUpdate = false;
        }
        void CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer, Buffer* newObjectBuffer, size_t meshOffset = 0);
        void Update() override;
        void UpdateBuffers(Range updateRange);
        void UpdateMeshlets();
        void UpdateVertexPositions(std::shared_ptr<Model> model);
        void UpdateMeshTransform(uint32_t meshIndex);

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
        uint32_t GetBufferIndex() const
        {
            return currentBufferIndex;
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
        void (*MeshAppend)(std::string, uint32_t) = nullptr;
    private:
        Device* aDevice;
        std::vector<Buffer> vertexBuffers{};
        std::vector<Buffer> objectBuffers{};
        std::vector<Buffer> objectDataBuffers{};
        std::vector<Buffer> collisionBuffer{};
        size_t vertexCount = 0;
        std::vector<Buffer> indexBuffers{};
        size_t indexCount = 0;
        Buffer drawIndexedIndirectBuffer{};
        Buffer drawIndexedCountBuffer{};
        Buffer drawMeshIndirectBuffer{};
        Buffer drawMeshCountBuffer{};
        void createIndirectBuffers();
        std::vector<Mesh> meshes{};
        std::vector<VkDrawIndexedIndirectCommand> drawIndexedCommands{};
        uint32_t batchSize;
        std::vector<Range> updateQueue{};
        void applyVertexBufferUpdate(Model::Vertex* vertices, Model::Index* indices);
        bool needUpdate;
        bool commandBufferNeedUpdate = false;
        std::mutex _mutex;
        std::unordered_map<uint32_t, uint32_t> textureIdMap{};
        std::vector<VkDescriptorImageInfo> textureInfos;
        std::vector<Descriptor*> samplersDescriptors;
        void createSamplerDescriptor();
        Descriptor* vertexDescriptor{nullptr}, *meshDescriptor{nullptr};
        void createSSBODescriptor();
        void updateSSBODescriptor();
        void appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos);
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;
        uint32_t meshletCount = 0;
        uint32_t meshletIDCount = 0;
        std::vector<Buffer> meshletIDCountBuffers{};
        std::vector<Buffer> meshletIDBuffers{};
        std::vector<Buffer> meshletBuffers{};
        std::vector<Buffer> meshletVertexBuffers{};
        std::vector<Buffer> meshletIndexBuffers{};
        std::vector<Buffer> meshletCullingBuffers{};
        uint32_t numOfGroup = 64;
        uint8_t currentBufferIndex = 0;
        uint8_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;
        void updateAll();
    };
}
