#pragma once
#include "AnA_Device.h"

#include <GLFW/glfw3.h>
#include <cstddef>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    const int MAX_FRAMES_IN_FLIGHT = 2;

    class AnA_SwapChain
    {
    public:
        AnA_SwapChain(AnA_Device *&mDevice, VkSurfaceKHR &mSurface, GLFWwindow *&mWindow);
        ~AnA_SwapChain();

        VkResult AcquireNextImage(uint32_t* pImageIndex);

        VkResult SubmitCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t* pImageIndex);

        uint32_t CurrentFrame = 0;

        VkExtent2D GetExtend();

        VkFormat GetFormat();

        uint32_t GetImageCount();

        VkRenderPass &GetRenderPass();

        std::vector<VkFramebuffer> GetSwapChainFramebuffers();

        void RecreateSwapChain();
    private:
        AnA_Device* aDevice;
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