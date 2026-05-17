#include "Headers/Instance.hpp"
#include "Headers/Window.hpp"

#include <iostream>
#include <string.h>

using namespace AnA;

Instance::Instance()
{
    createInstance();
    setupDebugMessenger();
}

Instance::Instance(Window& window)
{
    createInstance();
    setupDebugMessenger();

    window.CreateWindowSurface(this);
    surface = window.GetSurface();
}

Instance::~Instance()
{
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

VkInstance &Instance::GetInstance()
{
    return instance;
}

void Instance::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("One or More ValidationLayer Not supported!");
    }
    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "AnA", VK_MAKE_VERSION(1, 0, 0), "AnA Engine",
        VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_4};

    auto glfwExtensions = getWindowExtensions();
    if (!checkExtensions(glfwExtensions))
    {
        throw std::runtime_error("One or More Extensions Not supported!");
    }
    VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                               nullptr,
                                               0,
                                               &appInfo,
                                               0,
                                               nullptr,
                                               static_cast<uint32_t>(glfwExtensions.size()),
                                               glfwExtensions.data()};

    if (enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Couldn't Create Vulkan Instance!");
    }
}

std::vector<const char*> Instance::getWindowExtensions()
{
    uint32_t count;
    auto windowExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
    std::vector<const char*> extensions(windowExtensions, windowExtensions + count);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool Instance::checkExtensions(std::vector<const char*> targetExtensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    bool extFound = false;
    for (const auto &targetExtension : targetExtensions)
    {
        for (const auto &extension : extensions)
        {
            if (strcmp(extension.extensionName, targetExtension) == 0)
            {
                extFound = true;
                break;
            }
        }

        if (!extFound)
            return false;
    }
    return true;
}

bool Instance::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            // std::cout<<layerProperties.layerName<<"\n";
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT ,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* )
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult Instance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Instance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

VkDebugUtilsMessengerEXT debugMessenger;
void Instance::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Couldn't Create DebugMessenger!");
}
