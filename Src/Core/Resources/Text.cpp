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
    vertexBuffer = Buffer(aDevice, sizeof(glm::vec2) * 6000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    vertexBuffer.Map();
    if (aDevice->MeshShaderSupport())
    {
        meshletBuffer = Buffer(aDevice, 128 * sizeof(MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletBuffer.Map();

        meshletIndexBuffer = Buffer(aDevice, 6000,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletIndexBuffer.Map();
    }
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
    createSSBODescriptor();
}

void Text::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex)
{
    shader.GetPipeline().Bind(commandBuffer);
    auto& sets = shader.GetDescriptorSets()[bufferIndex];
    sets[0] = vertexDescriptor->GetSets()[currentBufferIndex];
    sets[1] = charInfoDescriptor->GetSets()[currentBufferIndex];
    sets[2] = meshDescriptor->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3, sets.data(), 0, nullptr);
    glm::vec2 resolution = {float(commandBuffer.Extent.width), float(commandBuffer.Extent.height)};
    vkCmdPushConstants(commandBuffer, shader.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(glm::vec2),
        &resolution);
}

void Text::Draw(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Text::DrawIndirect(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawCommandBuffer.GetBuffer(), 0,
        countBuffers[currentBufferIndex].GetBuffer(), 0,
        1,
        sizeof(VkDrawMeshTasksIndirectCommandEXT));
}

void Text::Update()
{
    needUpdate = false;
    if (!aDevice->MeshShaderSupport())
    {
        return;
    }
    Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
    {
        this->updateAll();
    });
}

bool Text::NeedUpdate()
{
    return needUpdate;
}

uint32_t Text::Insert(const TextInfo& textInfo, uint32_t capacity)
{
    std::unique_lock<std::mutex> lock(_mutex);
    TextMapData textMapData = {textInfo, 0, std::max(capacity, uint32_t(textInfo.text.size()))};
    totalCharCount += textMapData.capacity;
    textMap.emplace(id, std::move(textMapData));
    needUpdate = true;
    return id++;
}

void Text::Remove(uint32_t id)
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = textMap.find(id);
    if (iter != textMap.end())
    {
        totalCharCount -= iter->second.capacity;
        textMap.erase(iter);
        needUpdate = true;
    }
}

void Text::UpdateLayout(uint32_t id)
{
    if (!aDevice->MeshShaderSupport())
        return;
    _mutex.lock();
    auto& textMapData = textMap[id];
    TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[currentBufferIndex].GetMappedData());
    textBuffer[textMapData.index].size = textMapData.textInfo.size;
    textBuffer[textMapData.index].offset = textMapData.textInfo.offset * 2.0f;
    textBuffer[textMapData.index].color = textMapData.textInfo.color;
    _mutex.unlock();
}

void Text::UpdateText(uint32_t id, const std::string& text)
{
    auto& textMapData = textMap[id];
    textMapData.textInfo.text = text;
    if (uint32_t(text.length()) > textMapData.capacity)
    {
        textMapData.capacity = uint32_t(text.length());
        Update();
    }
    else
    {
        _mutex.lock();
        TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[currentBufferIndex].GetMappedData());
        CharacterInfo* chInfoBuffer = reinterpret_cast<CharacterInfo*>(charInfoBuffers[currentBufferIndex].GetMappedData());
        uint32_t chOffset = textBuffer[textMapData.index].chOffset;
        size_t meshletOffset = meshlets.size();
        for (size_t i = 0; i < textMapData.textInfo.text.size(); i++)
        {
            auto& chInfo = chInfoBuffer[chOffset + i];
            auto iter = characterMap.find(textMapData.textInfo.text[i]);
            if (iter == characterMap.end())
            {
                iter = characterMap.emplace(textMapData.textInfo.text[i], char(meshlets.size())).first;
                meshlets.push_back(textMapData.textInfo.text[i]);
            }
            chInfo.ch = iter->second;
            chInfo.index = uint32_t(i);
        }
        updateMeshlets(meshletOffset);
        textBuffer[textMapData.index].count = uint32_t(textMapData.textInfo.text.length());
        _mutex.unlock();
    }
}

TextInfo* Text::GetInfoById(uint32_t id)
{
    auto iter = textMap.find(id);
    return iter == textMap.end() ? nullptr : &iter->second.textInfo;
}

uint32_t Text::GetTextCount()
{
    return uint32_t(textMap.size());
}

void Text::ResetLayout()
{
    _mutex.lock();
    TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[currentBufferIndex].GetMappedData());
    size_t i = 0;
    for (auto& text : textMap)
    {
        textBuffer[i++].size = 0.0f;
        text.second.textInfo.visible = false;
    }
    _mutex.unlock();
}

void Text::updateAll()
{
    std::unique_lock<std::mutex> lock(_mutex);
    uint32_t index = 0, chIndex, textIndex = 0;
    if (charInfoBuffers[nextIndex].GetSize() < totalCharCount * sizeof(CharacterInfo))
    {
        charInfoBuffers[nextIndex].Resize(totalCharCount * sizeof(CharacterInfo));
    }
    if (textBuffers[nextIndex].GetSize() < textMap.size() * sizeof(TextData))
    {
        textBuffers[nextIndex].Resize(textMap.size() * sizeof(TextData));
    }

    CharacterInfo* chInfoBuffer = reinterpret_cast<CharacterInfo*>(charInfoBuffers[nextIndex].GetMappedData());
    TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[nextIndex].GetMappedData());
    auto meshletOffset = meshlets.size();
    for (auto& iter : textMap)
    {
        iter.second.index = textIndex;
        auto& textInfo = iter.second.textInfo;
        chIndex = 0;
        textBuffer[textIndex].size = textInfo.visible ? textInfo.size : 0.0f;
        textBuffer[textIndex].offset = textInfo.offset * 2.0f;
        textBuffer[textIndex].color = textInfo.color;
        textBuffer[textIndex].scissor = textInfo.scissor;
        textBuffer[textIndex].chOffset = index;
        textBuffer[textIndex].count = uint32_t(textInfo.text.length());
        for (auto& ch : textInfo.text)
        {
            //assert(ch < char(resourceManager->Characters.size()));
            auto& chInfo = chInfoBuffer[index + chIndex];
            auto iter = characterMap.find(ch);
            if (iter == characterMap.end())
            {
                iter = characterMap.emplace(ch, char(meshlets.size())).first;
                meshlets.push_back(ch);
            }
            chInfo.ch = iter->second;
            chInfo.index = chIndex;
            chIndex++;
        }
        index += iter.second.capacity;
        textIndex++;
    }
    updateMeshlets(meshletOffset);
    glm::uvec3* drawCommand = reinterpret_cast<glm::uvec3*>(drawCommandBuffer.GetMappedData());
    *drawCommand = glm::uvec3(uint32_t(textMap.size()), 1, 1);
    *reinterpret_cast<uint32_t*>(countBuffers[nextIndex].GetMappedData()) = textIndex ? 1u : 0u;

    updateSSBODescriptor();
}

void Text::updateMeshlets(size_t meshletOffset)
{
    auto characters = Resource::ResourceManager::GetCurrent()->Characters;
    auto meshletInfos = reinterpret_cast<MeshletInfo*>(meshletBuffer.GetMappedData());
    auto vertices = reinterpret_cast<glm::vec2*>(vertexBuffer.GetMappedData());
    auto meshletIndices = reinterpret_cast<uint8_t*>(meshletIndexBuffer.GetMappedData());
    for (size_t i = meshletOffset; i < meshlets.size(); i++)
    {
        auto& ch = characters[meshlets[i]];
        for (size_t j = 0; j < ch.vertices.size(); j++)
        {
            vertices[meshletVertexCount + j] = ch.vertices[j];
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
        descriptors[0].GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MAX_FRAMES_IN_FLIGHT,
        1,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    auto& charInfoDescriptorSetLayout =
        descriptors[1].GetLayout();
    charInfoDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MAX_FRAMES_IN_FLIGHT * 2,
        2,
        charInfoDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (aDevice->MeshShaderSupport())
    {
        auto& meshDescriptorSetLayout = descriptors[2].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MAX_FRAMES_IN_FLIGHT * 3,
            3,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    updateSSBODescriptor();
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
        bufferInfo.buffer = meshletBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
        bufferInfo.buffer = meshletIndexBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletIndexBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
    }

    currentBufferIndex = nextIndex;
    nextIndex = (currentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}
