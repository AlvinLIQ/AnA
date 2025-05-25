#include "Headers/Text.hpp"
#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

uint32_t id = 0;

Text::Text(Device* mDevice) : aDevice{mDevice}
{
    drawCommandBuffer = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawCommandBuffer.Map();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        charInfoBuffers[i] = Buffer(aDevice, sizeof(CharacterInfo) * 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        charInfoBuffers[i].Map();

        textBuffers[i] = Buffer(aDevice, sizeof(TextData) * 10, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        textBuffers[i].Map();

        countBuffers[i] = Buffer(aDevice, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        countBuffers[i].Map();
        *reinterpret_cast<uint32_t*>(countBuffers[i].GetMappedData()) = 0u;
    }
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
    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Text::DrawIndirect(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawCommandBuffer.GetBuffer(), 0,
        countBuffers[currentBufferIndex].GetBuffer(), 0,
        drawCount,
        sizeof(VkDrawMeshTasksIndirectCommandEXT));
}

void Text::Update()
{
    uint32_t index = 0, chIndex, textIndex = 0;
    if (charInfoBuffers[nextIndex].GetSize() < totalTextLen * sizeof(CharacterInfo))
    {
        charInfoBuffers[nextIndex].Resize(totalTextLen * sizeof(CharacterInfo));
    }
    if (textBuffers[nextIndex].GetSize() < textInfos.size() * sizeof(TextData))
    {
        textBuffers[nextIndex].Resize(textInfos.size() * sizeof(TextData));
    }

    CharacterInfo* chInfoBuffer = reinterpret_cast<CharacterInfo*>(charInfoBuffers[nextIndex].GetMappedData());
    TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[nextIndex].GetMappedData());
    for (auto& textInfo : textInfos)
    {
        chIndex = 0;
        textBuffer[textIndex].offset = textInfo.offset;
        textBuffer[textIndex].scale = textInfo.scale;
        for (auto& ch : textInfo.text)
        {
            //assert(ch < char(resourceManager->Characters.size()));
            auto& chInfo = chInfoBuffer[index];
            chInfo.ch = ch;
            chInfo.index = chIndex;
            chInfo.textIndex = textIndex;
            chIndex++;
            index++;
        }
        textIndex++;
    }
    glm::uvec3* drawCommand = reinterpret_cast<glm::uvec3*>(drawCommandBuffer.GetMappedData());
    *drawCommand = glm::uvec3(index, 1, 1);
    *reinterpret_cast<uint32_t*>(countBuffers[nextIndex].GetMappedData()) = totalTextLen ? 1u : 0u;

    currentBufferIndex = nextIndex;
    nextIndex = (currentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    needUpdate = false;
}

bool Text::NeedUpdate()
{
    return needUpdate;
}

uint32_t Text::Insert(const TextInfo& textInfo)
{
    textMap.emplace(id, uint32_t(textInfos.size()));
    textInfos.push_back(textInfo);
    needUpdate = true;
    return id++;
}

void Text::createSSBODescriptor()
{
    auto& shaders = Resource::ResourceManager::GetCurrent()->Shaders;
    auto& vertexDescriptorSetLayout =
        shaders[6].GetDescriptors()[DEFAULT_VERTEX_LAYOUT].GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MaxBatchSize,
        2,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    
    charInfoDescriptor = new Descriptor(aDevice, charInfoBuffers, charInfoBuffers[0].GetSize(),
        0, 1, 1000, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (aDevice->MeshShaderSupport())
    {
        auto& meshDescriptorSetLayout = shaders.back().GetDescriptors()[DEFAULT_MESHLET_LAYOUT].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MaxBatchSize * 3 + 1,
            4,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
}