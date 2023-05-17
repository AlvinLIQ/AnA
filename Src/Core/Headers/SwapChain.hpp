#pragma once
#include "Device.hpp"

#include <GLFW/glfw3.h>
#include <cstddef>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <vulkan/vulkan_core.h>

//#define INCLUDE_STB_IMAGE

#define MAX_FRAMES_IN_FLIGHT 2

namespace AnA
{
    class SwapChain
    {
    public:
        SwapChain(Device& mDevice, VkSurfaceKHR &mSurface, GLFWwindow* mWindow);
        ~SwapChain();

        VkResult AcquireNextImage(uint32_t* pImageIndex);

        VkResult SubmitCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t* pImageIndex);

        uint32_t CurrentFrame = 0;

        VkExtent2D GetExtent();

        VkFormat GetFormat();

        uint32_t GetImageCount();

        VkRenderPass &GetRenderPass();

        std::vector<VkFramebuffer> GetSwapChainFramebuffers();

        void RecreateSwapChain();

        void CreateImage(VkImageCreateInfo* pCreateInfo, VkImage* pImage, VkDeviceMemory* pImageMemory);
        #ifdef INCLUDE_STB_IMAGE
        void CreateTextureImage(const char* imagePath, VkImage* pTexImage, VkDeviceMemory* pTexMemory);
        #endif
    private:
        Device& aDevice;
        VkSurfaceKHR surface;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        GLFWwindow* window;
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        void createSwapChain();

        std::vector<VkImageView> swapChainImageViews;
        void createImageViews();

        VkFormat swapChainDepthFormat;
        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        void createDepthResources();

        VkRenderPass renderPass;
        void createRenderPass();

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