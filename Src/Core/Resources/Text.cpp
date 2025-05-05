#include "Headers/Text.hpp"
#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Text::Text(Device* mDevice) : aDevice{mDevice}
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        charInfoBuffers[i] = Buffer(aDevice, sizeof(CharacterInfo) * 1000, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        charInfoBuffers[i].Map(0, charInfoBuffers[i].GetSize());

        countBuffers[i] = Buffer(aDevice, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        countBuffers[i].Map(0, sizeof(uint32_t));
        *reinterpret_cast<uint32_t*>(countBuffers[i].GetMappedData()) = 0u;
    }
    drawCommands.resize(1000);
    charInfoDescriptor = new Descriptor(aDevice, charInfoBuffers, charInfoBuffers[0].GetSize(), 
        0, 1, 1000, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

Text::~Text()
{
    if (charInfoDescriptor)
        delete charInfoDescriptor;
}

void Text::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex)
{
    shader.GetPipeline().Bind(commandBuffer);
    auto& sets = shader.GetDescriptorSets()[bufferIndex];
    sets[1] = charInfoDescriptor->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        shader.GetPipelineLayout(), 0, 2, sets.data(), 0, nullptr);
}

void Text::Draw(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndexedIndirectCount(commandBuffer, drawCommandBuffers[currentBufferIndex].GetBuffer(), 0, 
        countBuffers[currentBufferIndex].GetBuffer(), 0, 
        drawCount, 
        sizeof(VkDrawIndexedIndirectCommand));
}

void Text::DrawIndirect(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndexedIndirectCount(commandBuffer, drawCommandBuffers[currentBufferIndex].GetBuffer(), 0, 
        countBuffers[currentBufferIndex].GetBuffer(), 0, 
        drawCount, 
        sizeof(VkDrawIndexedIndirectCommand));
}

void Text::Update()
{
    drawCommands.clear();
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    uint32_t index = 0;
    for (auto& textInfo : TextMap)
    {
        for (auto& ch : textInfo.second.text)
        {
            assert(ch < char(resourceManager->Characters.size()));
            VkDrawIndexedIndirectCommand& drawIndexedCommand = drawCommands[index];
            drawIndexedCommand.vertexOffset = 0;
            drawIndexedCommand.firstIndex = resourceManager->Characters[ch].indexOffset;
            drawIndexedCommand.indexCount = uint32_t(resourceManager->Characters[ch].indices.size());
            drawIndexedCommand.firstInstance = index;
            index++;
        }
    }

    currentBufferIndex = nextIndex;
    nextIndex = (currentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool Text::NeedUpdate()
{
    return true;
}