#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include "Renderable.hpp"
#include "Mesh.hpp"
#include "CommandBuffer.hpp"
#include "../../Headers/Buffer.hpp"
#include "../../Headers/Types.hpp"

namespace AnA
{
    struct MeshObject
    {
        AnA::Transform transform;
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t textureId{};
        uint32_t meshId;
        AnimationInfo animationInfo{};
    };

    struct MeshletInfo
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t vertexCount;
        uint32_t indexCount;
    };

    struct MeshBufferObject
    {
        glm::vec3 center;
        float radius;
        glm::vec3 halfVolume;
        uint32_t textureId;
        glm::mat4 transform;
        VkDeviceAddress vertexPtr;
        VkDeviceAddress meshletVertexPtr;
        VkDeviceAddress meshletIndexPtr;
        VkDeviceAddress indexPtr;
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
        uint32_t textureId{};
    };

    struct FrustumPlanes
    {
        glm::vec4 planes[6];
        static void ExtractFrustumPlanes(const glm::mat4& m, FrustumPlanes& fp);
    };

    struct ObjectData
    {
        uint32_t objectCount;
        uint32_t collidedCount;
        uint32_t meshletCount;
    };

    struct CollisionData
    {
        glm::uvec2 pair;
        glm::vec3 normal;
        float penetration;
    };

    struct MeshPushConstant
    {
        VkDeviceAddress meshPtr;
        VkDeviceAddress meshletPtr;
        VkDeviceAddress meshletIDPtr;
        VkDeviceAddress miscPtr;
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
        void Append(std::vector<Mesh::Vertex>& vertices, std::vector<uint32_t>& indices, Transform transform = {}, uint32_t textureId = 0);
        void Append(std::vector<Mesh::Vertex>& vertices, Transform transform = {});
        void RemoveAt(uint32_t meshIndex);
        void RemoveAt(Range removeRange);
        void RemoveAt(std::vector<uint32_t> meshIndices);
        void Bind(CommandBuffer& commandBuffer, Shader& shader) override;
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
        void CommitBufferUpdate(Buffer* newObjectBuffer, size_t meshOffset = 0);
        void Update() override;
        void UpdateBuffers(Range updateRange);
        void UpdateMeshlets();
        void UpdateVertexPositions(std::shared_ptr<Mesh> model);
        void UpdateMeshTransform(uint32_t meshIndex);

        MeshObject& At(size_t index)
        {
            return meshes[index];
        }
        const MeshObject* Get() const
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
        uint32_t GetObjectCount() const
        {
            return uint32_t(meshes.size());
        }
        uint32_t GetMeshletCount() const
        {
            return meshletCount;
        }
        uint32_t GetMeshletIDCount() const
        {
            return meshletIDCount;
        }
        void (*MeshAppend)(std::string, uint32_t) = nullptr;
    private:
        Device* aDevice;
        std::vector<Buffer> meshBuffers{};
        std::vector<Buffer> collisionBuffer{};
        std::vector<Buffer> drawMeshIndirectBuffers{};
        Buffer drawMeshCountBuffer{};
        void createIndirectBuffers();
        std::vector<MeshObject> meshes{};
        std::vector<VkDrawIndexedIndirectCommand> drawIndexedCommands{};
        uint32_t batchSize;
        std::vector<Range> updateQueue{};
        bool needUpdate;
        bool commandBufferNeedUpdate = false;
        std::mutex _mutex;
        uint32_t meshletCount = 0;
        uint32_t meshletIDCount = 0;
        std::vector<Buffer> meshletIDBuffers{};

        uint32_t numOfGroup = 64;
        uint8_t currentBufferIndex = 0;
        uint8_t nextIndex = 1 % MAX_FRAMES_IN_FLIGHT;

        MeshPushConstant meshPushConstant;
        void updateAll();

        friend class Animations;
    };
}
