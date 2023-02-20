#pragma once

#include "AnA_Buffer.h"
#include "AnA_Device.h"
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

            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };

        AnA_Model(AnA_Device *&mDevice, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
        ~AnA_Model();
        
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint16_t> &indices);

        AnA_Device *&aDevice;
        AnA_Buffer *vertexBuffer;
        AnA_Buffer *indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
    };
}