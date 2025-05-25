#pragma once
#include "Device.hpp"
#include "Types.hpp"

#include <GLFW/glfw3.h>
#include <vector>

#define INCLUDE_STB_IMAGE

#define NextFrameIndex(X) ((X + 1) % MAX_FRAMES_IN_FLIGHT)
#define SHADOW_MAP_DIM 4096

#define ScaleX Scale[0]
#define ScaleY Scale[1]

namespace AnA
{
    class SwapChain
    {
    public:
        SwapChain(Device* mDevice, VkSurfaceKHR &mSurface, GLFWwindow* mWindow);
        ~SwapChain();

        Float Scale[2];

        VkResult AcquireNextImage();

        VkResult SubmitCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t commandBufferCount);

        uint32_t CurrentFrame = 1;
        uint32_t CurrentImage = 0;

        static SwapChain* GetCurrent();

        VkExtent2D& GetExtent();

        VkFormat GetFormat();
        VkFormat GetDepthFormat();

        uint32_t GetImageCount();

        VkRenderPass& GetRenderPass();
        VkRenderPass& GetOffscreenRenderPass();

        VkFramebuffer& GetCurrentFramebuffer()
        {
            return swapChainFramebuffers[CurrentImage];
        }

        void SetViewport(CommandBuffer& commandBuffer);
        void SetViewport(CommandBuffer& commandBuffer, VkOffset2D& offset);
        static void SetViewport(CommandBuffer& commandBuffer, VkExtent2D& extent);
        static void SetViewport(CommandBuffer& commandBuffer, VkOffset2D& offset, VkExtent2D& extent);

        VkSemaphore& GetCurrentSemaphore();

        std::vector<VkFramebuffer> GetSwapChainFramebuffers();

        void RecreateSwapChain();

        Device* GetDevice();
    private:
        Device* aDevice;
        VkSurfaceKHR surface;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        GLFWwindow* window;
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        VkViewport viewport;
        VkRect2D scissor;
        void createSwapChain();

        std::vector<VkImageView> swapChainImageViews;
        void createImageViews();

        VkFormat swapChainDepthFormat;
        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        VkSampleCountFlagBits msaaSamplers;

        VkImage colorImage;
        VmaAllocation colorImageAllocation;
        VkImageView colorImageView;
        void createColorResources();
        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImageAllocations;
        std::vector<VkImageView> depthImageViews;
        void createDepthResources();

        VkRenderPass renderPass;
        void createRenderPass();
        VkRenderPass offscreenRenderPass;
        void createOffscreenRenderPass();

        std::vector<VkFramebuffer> swapChainFramebuffers;
        void createFramebuffers();

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        void createSyncObjects();

        void cleanupSwapChain();
    };
}
