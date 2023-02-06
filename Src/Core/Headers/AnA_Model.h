#pragma once

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
            glm::vec3 postion;

            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };

        AnA_Model(AnA_Device *&mDevice, const std::vector<Vertex> &vertices);
        ~AnA_Model();
        
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        AnA_Device *&aDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}