#pragma once
#include "../../Headers/Object.hpp"
#include "../../Headers/Pipeline.hpp"
#include "../../Headers/SwapChain.hpp"
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Buffer.hpp"
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>

#define TRIANGLE_LIST_PIPELINE 0
#define LINE_LIST_PIPELINE 1
#define POINT_LIST_PIPELINE 2
#define COMPUTE_PIPELINE 3

#define PIPELINE_COUNT 4

namespace AnA
{
    namespace RenderSystems
    {
        struct CameraBufferObject
        {
            glm::mat4 proj{1.f};
            glm::mat4 view{1.f};
            glm::vec2 resolution{};
            static VkDescriptorSetLayoutBinding GetBindingDescriptionSet();
            static VkDescriptorBufferInfo GetBufferInfo(VkBuffer camBuffer);
        };
        
        class RenderSystem
        {
        public:
            RenderSystem(Device& mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, Objects &objects, int pipeLineIndex = TRIANGLE_LIST_PIPELINE);
            void UpdateCameraBuffer(Cameras::Camera &camera);
            static RenderSystem* GetCurrent();

            std::array<VkDescriptorSetLayout, 3>& GetDescriptorSetLayouts();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;
            
            VkPipelineLayout pipelineLayout;
            VkPipelineLayout computePipelineLayout;
            void createPipelineLayouts();

            std::array<Pipeline*, PIPELINE_COUNT> pipelines{};

            std::vector<Buffer*> cameraBuffers;
            void createCameraBuffers();

            VkDescriptorPool descriptorPool;

            std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = {};
            VkDescriptorSetLayout computeDescriptorSetLayout;
            void createDescriptorSetLayout();
            std::vector<VkDescriptorSet> descriptorSets;
        };
    }
}