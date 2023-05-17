#pragma once

#include "Buffer.hpp"
#include "Device.hpp"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

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

        Model(Device *&mDevice, const ModelInfo &modelInfo);
        ~Model();

        static void CreateModelFromFile(Device *&mDevice, const char *filePath, std::shared_ptr<Model> &model);
        
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<Index> &indices);

        Device *&aDevice;
        Buffer *vertexBuffer;
        bool hasIndexBuffer;
        Buffer *indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
        Index indexStep;
    };
}
