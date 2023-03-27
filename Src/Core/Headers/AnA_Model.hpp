#pragma once

#include "AnA_Buffer.hpp"
#include "AnA_Device.hpp"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace AnA
{
    class AnA_Model
    {
    public:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal{};
            glm::vec3 uv{};

            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };
        typedef uint32_t Index;
        
        struct ModelInfo
        {
            std::vector<Vertex> vertices;
            Index indexStep;
            std::vector<Index> indices;
        };

        AnA_Model(AnA_Device *&mDevice, const ModelInfo &modelInfo);
        ~AnA_Model();

        static std::unique_ptr<AnA_Model> CreateModelFromFile(AnA_Device *&mDevice, const char *filePath);
        
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<Index> &indices);

        AnA_Device *&aDevice;
        AnA_Buffer *vertexBuffer;
        bool hasIndexBuffer;
        AnA_Buffer *indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
        Index indexStep;
    };
}