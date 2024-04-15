#include "Headers/RenderSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::Systems;
using namespace AnA::Cameras;

RenderSystem* currentRenderSystem = nullptr;

RenderSystem::RenderSystem(Device& mDevice, SwapChain& mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    currentRenderSystem = this;
}

RenderSystem::~RenderSystem()
{

}

RenderSystem* RenderSystem::GetCurrent()
{
    return currentRenderSystem;
}

void RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, Objects &objects, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[aSwapChain.CurrentFrame];
    Object* object;
    auto objectArray = objects.Get();
    for (int i = 0; i < objectArray.size(); i++)
    {
        object = objectArray[i];
        if (object->Texture == nullptr)
        {
            uint32_t color = (uint32_t)0xFF000000 ^ ((uint32_t)(object->Color.b * 255.0f) << 16) ^ ((uint32_t)(object->Color.g * 255.0f) << 8) ^ ((uint32_t)(object->Color.r * 255.0f));
            object->Texture = std::make_unique<Texture>(color, aDevice);
        }
        //sets[DEFAULT_SAMPLER_LAYOUT] = object->Texture->GetDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
            sets.data(), 0, nullptr);

        object->Model->Bind(commandBuffer);

        object->Model->Draw(commandBuffer, i);
    }
}

void RenderSystem::RenderMeshes(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];
    auto& textureMap = resourceManager->TextureMap;
    auto& texture = textureMap.at(DEFAULT_TEXTURE_ID);
    //sets[DEFAULT_SAMPLER_LAYOUT] = texture.GetDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer);
}

void RenderSystem::RenderBatch(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, size_t index)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];
    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer, sets, shader.GetPipelineLayout(), 0, meshes.GetMeshCount());
}
