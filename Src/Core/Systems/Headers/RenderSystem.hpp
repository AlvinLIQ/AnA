#pragma once
#include "../../Headers/Object.hpp"
#include "../../Headers/Pipeline.hpp"
#include "../../Headers/SwapChain.hpp"
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Buffer.hpp"
#include "../../Headers/Descriptor.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace Systems
    {
        struct CameraBufferObject
        {
            glm::mat4 proj{1.f};
            glm::mat4 view{1.f};
            glm::mat4 invView{1.f};
            glm::vec2 resolution{};
        };
        
        class RenderSystem
        {
        public:
            RenderSystem(Device& mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, Objects &objects, int pipeLineIndex = TRIANGLE_LIST_PIPELINE);
            void UpdateCameraBuffer(Cameras::Camera &camera);
            Pipelines* GetPipelines() const
            {
                return pipelines;
            }
            VkDescriptorSet& GetDescriptorSet()
            {
                return descriptor->GetSets()[aSwapChain.CurrentFrame];
            }
            static RenderSystem* GetCurrent();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;

            Pipelines* pipelines;

            std::vector<Buffer*> cameraBuffers;
            void createCameraBuffers();

            Descriptor* descriptor;
        };
    }
}