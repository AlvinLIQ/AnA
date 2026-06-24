#pragma once

#include "Device.hpp"
#include "Window.hpp"
#include "SwapChain.hpp"
#include "../Resources/Headers/CommandBuffer.hpp"
#include "../Resources/Headers/Renderable.hpp"

#include <cassert>
#include <volk.h>

namespace AnA
{
    class Renderer
    {
    public:
        Renderer(Window& mWindow, Device* mDevice);
        ~Renderer();

        uint32_t GetFrameIndex() const
        {
            return aSwapChain->CurrentFrame;
        }

        bool IsFrameInProgress() const
        {
            return isFrameStarted;
        }

        bool NeedUpdate() const
        {
            return needUpdate;
        }

        VkCommandBuffer GetCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress!");
            return commandBuffers.Get();
        }

        SwapChain& GetSwapChain()
        {
            return *aSwapChain;
        }
        VkExtent2D GetSwapChainExtent() const
        {
            return aSwapChain->GetExtent();
        }
        Float GetAspect() const
        {
            auto swapChainextent = GetSwapChainExtent();
            return static_cast<float>(swapChainextent.width) / static_cast<float>(swapChainextent.height);
        }
        Float GetGPUTime()
        {
            return gpuTime;
        }
        VkCommandBufferInheritanceInfo& GetInheritanceInfo(RenderPassType renderPassType)
        {
            return inheritanceInfos[renderPassType];
        }
        CommandBuffer* BeginFrame();

        void RecordOffscreenSecondaryCommandBuffer(RecordCallBack recordCallBack);
        void ExecuteOffscreenSecondaryCommandBuffer(CommandBuffer& commandBuffer);

        void Render(CommandBuffer& commandBuffer, Renderable& renderable, Shader& shader, uint32_t bufferIndex = 0);
        void RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, Shader& shader, uint32_t bufferIndex = 0);

        void EndFrame();
        void BeginRendering(CommandBuffer& commandBuffer);
        void EndRendering(CommandBuffer& commandBuffer);

        void EndRenderPass(CommandBuffer& commandBuffer);
        void BeginOffscreenRendering(CommandBuffer& commandBuffer);
        void EndOffscreenRendering(CommandBuffer& commandBuffer);
    private:
        Window& aWindow;
        Device* aDevice;
        SwapChain* aSwapChain;

        CommandBuffer commandBuffers;
        CommandBuffer offscreenSecondaryCommandBuffers;

        static constexpr uint32_t queriesPerFrame = 2;
        uint32_t firstQuery = 0;
        uint64_t timestamps[queriesPerFrame * MAX_FRAMES_IN_FLIGHT] = {1, 1, 1, 1};
        Float gpuTime = 0.0f;
        VkQueryPool timestampQueryPool {VK_NULL_HANDLE};
        void createTimestampQueryPool();

        bool isFrameStarted = false;

        bool needUpdate = false;
        VkClearValue clearValues[2] = {{.color = {{1.0f, 1.0f, 1.0f, 1.0f}}}, {.depthStencil = {1.0f, 0}}};
        VkCommandBufferInheritanceInfo inheritanceInfos[RENDER_PASS_TYPE_SIZE]{};
    };
}
