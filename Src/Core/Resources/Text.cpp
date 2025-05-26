#include "Headers/Text.hpp"
#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

uint32_t id = 0;

Text::Text(Device* mDevice) : aDevice{mDevice}
{
    
}

Text::~Text()
{
    if (charInfoDescriptor)
        delete charInfoDescriptor;
    if (vertexDescriptor)
        delete vertexDescriptor;
    if (meshDescriptor)
        delete meshDescriptor;
}

void Text::Init()
{
    drawCommandBuffer = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawCommandBuffer.Map();
    vertexBuffer = Buffer(aDevice, sizeof(glm::vec2) * 3000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    vertexBuffer.Map();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        charInfoBuffers[i] = Buffer(aDevice, sizeof(CharacterInfo) * 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        charInfoBuffers[i].Map();

        textBuffers[i] = Buffer(aDevice, sizeof(TextData) * 10, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        textBuffers[i].Map();

        countBuffers[i] = Buffer(aDevice, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        countBuffers[i].Map();
        *reinterpret_cast<uint32_t*>(countBuffers[i].GetMappedData()) = 0u;
        if (aDevice->MeshShaderSupport())
        {
            meshletBuffers[i] = Buffer(aDevice, 100 * sizeof(MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

            meshletBuffers[i].Map();

            meshletVertexBuffers[i] = Buffer(aDevice, 500 * sizeof(uint32_t),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

            meshletVertexBuffers[i].Map();

            meshletIndexBuffers[i] = Buffer(aDevice, 1000,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

            meshletIndexBuffers[i].Map();
        }
    }
    createSSBODescriptor();
}

void Text::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex)
{
    shader.GetPipeline().Bind(commandBuffer);
    auto& sets = shader.GetDescriptorSets()[bufferIndex];
    sets[0] = vertexDescriptor->GetSets()[0];
    sets[1] = charInfoDescriptor->GetSets()[currentBufferIndex];
    sets[2] = meshDescriptor->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3, sets.data(), 0, nullptr);
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
    auto meshletOffset = meshlets.size();
    for (auto& textInfo : textInfos)
    {
        chIndex = 0;
        textBuffer[textIndex].offset = textInfo.offset;
        textBuffer[textIndex].scale = textInfo.scale;
        for (auto& ch : textInfo.text)
        {
            //assert(ch < char(resourceManager->Characters.size()));
            auto& chInfo = chInfoBuffer[index];
            auto iter = characterMap.find(ch);
            if (iter == characterMap.end())
            {
                iter = characterMap.emplace(ch, char(meshlets.size())).first;
                meshlets.push_back(ch);
            }
            chInfo.ch = iter->second;
            chInfo.index = chIndex;
            chIndex++;
            index++;
        }
        textIndex++;
    }
    updateMeshlets(meshletOffset);
    glm::uvec3* drawCommand = reinterpret_cast<glm::uvec3*>(drawCommandBuffer.GetMappedData());
    *drawCommand = glm::uvec3(index, 1, 1);
    *reinterpret_cast<uint32_t*>(countBuffers[nextIndex].GetMappedData()) = totalTextLen ? 1u : 0u;

    updateSSBODescriptor();
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

void Text::updateMeshlets(size_t meshletOffset)
{
    auto characters = Resource::ResourceManager::GetCurrent()->Characters;
    auto meshletInfos = reinterpret_cast<MeshletInfo*>(vertexBuffer.GetMappedData());
    auto vertices = reinterpret_cast<glm::vec2*>(vertexBuffer.GetMappedData());
    auto meshletVertices = reinterpret_cast<uint32_t*>(meshletVertexBuffers[nextIndex].GetMappedData());
    auto meshletIndices = reinterpret_cast<uint8_t*>(meshletIndexBuffers[nextIndex].GetMappedData());
    for (size_t i = meshletOffset; i < meshlets.size(); i++)
    {
        auto& ch = characters[meshlets[i]];
        for (size_t j = 0; j < ch.vertices.size(); j++)
        {
            vertices[meshletVertexCount + j] = ch.vertices[j];
            meshletVertices[meshletVertexCount + j] = meshletVertexCount + j;
        }
        for (size_t j = 0; j < ch.indices.size(); j++)
        {
            meshletIndices[meshletIndexCount + j] = ch.indices[j];
        }
        meshletInfos[i].vertexOffset = meshletVertexCount;
        meshletInfos[i].indexOffset = meshletIndexCount; 
        meshletInfos[i].vertexCount = uint32_t(ch.vertices.size());
        meshletInfos[i].indexCount = uint32_t(ch.indices.size());
        meshletVertexCount += uint32_t(ch.vertices.size());
        meshletIndexCount += uint32_t(ch.indices.size());
    }
}

void Text::createSSBODescriptor()
{
    auto& shaders = Resource::ResourceManager::GetCurrent()->Shaders;
    auto& descriptors = shaders[6].GetDescriptors();
    auto& vertexDescriptorSetLayout =
        descriptors[DEFAULT_VERTEX_LAYOUT].GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MaxBatchSize,
        1,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    auto& charInfoDescriptorSetLayout =
        descriptors[1].GetLayout();
    charInfoDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MaxBatchSize * 2,
        2,
        charInfoDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (aDevice->MeshShaderSupport())
    {
        auto& meshDescriptorSetLayout = descriptors[2].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MaxBatchSize * 3,
            3,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
}

void Text::updateSSBODescriptor()
{
    //Vertex Buffer
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffer.GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vertexDescriptor->GetSets()[nextIndex]);

    //Char Buffer
    bufferInfo.buffer = charInfoBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = charInfoBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        charInfoDescriptor->GetSets()[nextIndex]);
    bufferInfo.buffer = textBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = textBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        charInfoDescriptor->GetSets()[nextIndex]);

    if (aDevice->MeshShaderSupport())
    {
        //Meshlet Buffer
        bufferInfo.buffer = meshletBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
        bufferInfo.buffer = meshletVertexBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletVertexBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
        bufferInfo.buffer = meshletIndexBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletIndexBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
    }
}