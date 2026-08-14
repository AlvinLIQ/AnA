#include "Headers/Text.hpp"
#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"
#include "Resources/Headers/Scene.hpp"

using namespace AnA;

uint32_t id = 0;

Text::Text(Device* mDevice) : aDevice{mDevice}
{

}

Text::~Text()
{
}

void Text::Init()
{
    vertexBuffer = Buffer(aDevice, sizeof(glm::vec2) * 6000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    vertexBuffer.Map();
    meshletBuffer = Buffer(aDevice, 128 * sizeof(MeshletInfo),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    meshletBuffer.Map();

    meshletIndexBuffer = Buffer(aDevice, 6000,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    meshletIndexBuffer.Map();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (aDevice->MeshShaderSupport())
        {
            drawCommandBuffers[i] = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            drawCommandBuffers[i].Map();
        }
        charInfoBuffers[i] = Buffer(aDevice, sizeof(CharacterInfo) * 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        charInfoBuffers[i].Map();

        textBuffers[i] = Buffer(aDevice, sizeof(TextData) * 10, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        textBuffers[i].Map();

        countBuffers[i] = Buffer(aDevice, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        countBuffers[i].Map();
        *reinterpret_cast<uint32_t*>(countBuffers[i].GetMappedData()) = 0u;
    }
}

void Text::Bind(CommandBuffer& commandBuffer, Shader& shader)
{
    shader.GetPipeline().Bind(commandBuffer);
    aDevice->vkCmdSetPolygonModeEXT(commandBuffer, PolygonMode);

    textPushConstant.vertexPtr = vertexBuffer.GetAddress();
    textPushConstant.charInfoPtr = charInfoBuffers[currentBufferIndex].GetAddress();
    textPushConstant.textDataPtr = textBuffers[currentBufferIndex].GetAddress();
    textPushConstant.meshletPtr = meshletBuffer.GetAddress();
    textPushConstant.meshIndexPtr = meshletIndexBuffer.GetAddress();
    textPushConstant.resolution = {float(commandBuffer.Extent.width), float(commandBuffer.Extent.height)};
    vkCmdPushConstants(commandBuffer, shader.GetPipelineLayout(),
        shader.StageFlags, 0, sizeof(textPushConstant),
        &textPushConstant);
}

void Text::Draw(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Text::DrawIndirect(CommandBuffer& commandBuffer)
{
    if (!meshlets.size())
        return;

    if (aDevice->MeshShaderSupport())
    {
        aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawCommandBuffers[currentBufferIndex].GetBuffer(), 0,
            countBuffers[currentBufferIndex].GetBuffer(), 0,
            1,
            sizeof(VkDrawMeshTasksIndirectCommandEXT));
    }
    else if (drawCommandBuffers[currentBufferIndex].GetBuffer())
    {
        vkCmdDrawIndirectCount(commandBuffer, drawCommandBuffers[currentBufferIndex].GetBuffer(), 0,
            countBuffers[currentBufferIndex].GetBuffer(), 0,
            uint32_t(drawCommandBuffers[currentBufferIndex].GetSize() / sizeof(VkDrawIndirectCommand)), sizeof(VkDrawIndirectCommand));
    }
}

void Text::Update()
{
    needUpdate = false;
    Resources::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
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
    _mutex.lock();
    auto& textMapData = textMap[id];
    TextData* textBuffer = reinterpret_cast<TextData*>(textBuffers[currentBufferIndex].GetMappedData());
    textBuffer[textMapData.index].size = textMapData.textInfo.visible ? textMapData.textInfo.size : 0.0f;
    textBuffer[textMapData.index].offset = textMapData.textInfo.offset * 2.0f;
    textBuffer[textMapData.index].color = textMapData.textInfo.color;
    textBuffer[textMapData.index].scissor = textMapData.textInfo.scissor;
    _mutex.unlock();
}

void Text::updateTextInfo(TextInfo& textInfo, uint32_t& chIndex, uint32_t& index, const uint32_t& textIndex,
    CharacterInfo* chInfoBuffer)
{
    int realChar = 0;
    auto characters = Resources::ResourceManager::GetCurrent()->Characters;
    for (size_t i = 0; i < textInfo.text.length(); i++)
    {
        //assert(ch < char(resourceManager->Characters.size()));
        int ch = int(textInfo.text[i]);
        if ((ch & 0x80) == 0x00)
            realChar = ch;
        else if ((ch & 0xE0) == 0xC0)
        {
            realChar =
                    ((ch & 0x1F) << 6) |
                    ((textInfo.text[i + 1] & 0x3F));
            i += 1;
        }
        else if ((ch & 0xF0) == 0xE0)
        {
            realChar =
                    ((ch & 0x0F) << 12) |
                    ((textInfo.text[i + 1] & 0x3F) << 6) |
                    (textInfo.text[i + 2] & 0x3F);
            i += 2;
        }
        else if ((ch & 0xF8) == 0xF0)
        {
            realChar =
                    ((ch & 0x07) << 18) |
                    ((textInfo.text[i + 1] & 0x3F) << 12) |
                    ((textInfo.text[i + 2] & 0x3F) << 6) |
                    (textInfo.text[i + 3] & 0x3F);
            i += 3;
        }
        auto& chInfo = chInfoBuffer[index + chIndex];
        auto iter = characterMap.find(realChar);
        if (iter == characterMap.end())
        {
            iter = characterMap.emplace(realChar, int(meshlets.size())).first;
            meshlets.push_back(realChar);
        }
        chInfo.ch = iter->second;
        chInfo.index = chIndex;
        if (!aDevice->MeshShaderSupport())
        {
            VkDrawIndirectCommand& drawCmd =
                ((VkDrawIndirectCommand*)drawCommandBuffers[nextIndex].GetMappedData())[index + chIndex];
            drawCmd.firstInstance = textIndex;
            drawCmd.firstVertex = 0;
            drawCmd.instanceCount = 1;
            drawCmd.vertexCount = uint32_t(characters[ch].indices.size());
        }
        chIndex++;
    }
    textInfo.length = chIndex;
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
        uint32_t index = 0;

        updateTextInfo(textMapData.textInfo, index, chOffset, textMapData.index, chInfoBuffer);
        textBuffer[textMapData.index].count = index;
        updateMeshlets(meshletOffset);
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
    if (aDevice->MeshShaderSupport())
    {
        glm::uvec3* drawCommand = reinterpret_cast<glm::uvec3*>(drawCommandBuffers[nextIndex].GetMappedData());
        *drawCommand = glm::uvec3(uint32_t(textMap.size()), 1, 1);
    }
    else if (drawCommandBuffers[nextIndex].GetSize() < totalCharCount * sizeof(VkDrawIndirectCommand))
    {
        drawCommandBuffers[nextIndex] =
            Buffer(aDevice, totalCharCount * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        drawCommandBuffers[nextIndex].Map();
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
        updateTextInfo(textInfo, chIndex, index, textIndex, chInfoBuffer);

        textBuffer[textIndex].count = chIndex;
        index += iter.second.capacity;
        textIndex++;
    }
    updateMeshlets(meshletOffset);

    uint32_t drawCount = 0;
    if (textIndex)
    {
       drawCount = aDevice->MeshShaderSupport() ? 1u : uint32_t(totalCharCount);
    }
    *reinterpret_cast<uint32_t*>(countBuffers[nextIndex].GetMappedData()) = drawCount;

    currentBufferIndex = nextIndex;
    nextIndex = NextFrameIndex(nextIndex);
}

void Text::updateMeshlets(size_t meshletOffset)
{
    auto characters = Resources::ResourceManager::GetCurrent()->Characters;
    auto meshletInfos = reinterpret_cast<MeshletInfo*>(meshletBuffer.GetMappedData());
    auto vertices = reinterpret_cast<glm::vec2*>(vertexBuffer.GetMappedData());
    auto meshletIndices = reinterpret_cast<uint8_t*>(meshletIndexBuffer.GetMappedData());

    for (size_t i = meshletOffset; i < meshlets.size(); i++)
    {
        std::unordered_map<int, Character>::iterator iter;

        iter = characters.find(meshlets[i]);
        if (iter == characters.end())
        {
            aDevice->BuildFontVertices(characters, meshlets[i], meshlets[i] + 1);
            iter = characters.find(meshlets[i]);
        }
        auto& ch = iter->second;
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
