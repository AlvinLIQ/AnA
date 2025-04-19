#pragma once

#include "../../Headers/Buffer.hpp"
#include <functional>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace AnA
{
    class Model
    {
    public:
        struct Vertex
        {
            glm::vec3 position{};
            alignas(8) glm::vec3 normal{};
            alignas(8) glm::vec2 uv{};
            alignas(8) glm::uint32_t textureId{};

            bool operator==(const Vertex& vertex) const
            {
                return position == vertex.position && normal == vertex.normal && uv == vertex.uv;
            }
            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };
        struct VertexHash
        {
            std::size_t operator() (const Vertex& vertex) const
            {
                return std::hash<float>()(vertex.position.x) ^ std::hash<float>()(vertex.position.y) ^ std::hash<float>()(vertex.position.z)
                    ^ std::hash<float>()(vertex.normal.x)
                    ^ std::hash<float>()(vertex.normal.y)
                    ^ std::hash<float>()(vertex.normal.z)
                    ^ std::hash<float>()(vertex.uv.x)
                    ^ std::hash<float>()(vertex.uv.y);
            }
        };

        struct BSPNode
        {
            BSPNode* left = nullptr;
            BSPNode* right = nullptr;
            std::vector<Vertex>* vertices = nullptr;
        };
        typedef uint32_t Index;
        
        struct ModelInfo
        {
            std::vector<Vertex> vertices;
            std::vector<glm::mat3> transforms;
            std::vector<glm::vec2> vertexProjections;
            Index indexStep;
            std::vector<Index> indices;
        };

        struct ModelStorageBufferObject
        {
            glm::mat4 model;
            static VkDescriptorSetLayoutBinding GetBindingDescriptionSet();
        };

        Model(Device* mDevice, const ModelInfo& modelInfo);
        ~Model();

        std::vector<Vertex>& GetVertices()
        {
            return vertices;
        }

        std::vector<Index>& GetIndices()
        {
            return indices;
        }

        std::vector<glm::mat3>& GetTransforms()
        {
            return transforms;
        }

        std::vector<glm::vec2>& GetVertexProjections()
        {
            return vertexProjections;
        }

        static void CreateModelFromFile(Device* mDevice, const char* filePath, std::shared_ptr<Model>& model);
        static void CreateMeshFromFile(const char *filePath, std::vector<Vertex>& vertices, std::vector<Index>& indices, size_t vertexOffset = 0);
        void LoadMaterialFromFile(const char* filePath);
        
        void Bind(VkCommandBuffer commandBuffer);
        void Bind(VkCommandBuffer commandBuffer, VkDeviceSize& vertexOffset, VkDeviceSize& indexOffset);
        void Draw(VkCommandBuffer commandBuffer, Index instanceIndex = 0);
    private:
        void createVertexBuffers();
        void createIndexBuffers();

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        std::vector<glm::mat3> transforms;
        std::vector<glm::vec2> vertexProjections;

        Device* aDevice;
        Buffer vertexBuffer;
        bool hasIndexBuffer;
        Buffer indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
        Index indexStep;
    };
}
