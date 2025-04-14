#include "Headers/Glyph.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

uint32_t glyphId = 0;

Glyphs::Glyphs(Device* mDevice) : aDevice{mDevice}
{
    glyphletInfoBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    buildGlyphlets();
}

Glyphs::~Glyphs()
{
    
}

void Glyphs::Append(const GlyphInfo& info, uint32_t& id)
{
    id = glyphId;
    glyphMap.insert(std::pair<uint32_t, GlyphInfo>(glyphId++, info));
    Update(id);
}

void Glyphs::Update(uint32_t id)
{
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    if (!glyphletDescriptor)
    {
        glyphletDescriptor = new Descriptor(aDevice, 
            MAX_FRAMES_IN_FLIGHT, 2, 
            resourceManager->Shaders[3].GetDescriptors()[0]->GetLayout(), 
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    if (!glyphletInfoBuffers[nextIndex].GetSize())
    {
        glyphletInfoBuffers[nextIndex] = Buffer(aDevice, sizeof(GlyphInfo) * glyphMap.size(), 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        glyphletInfoBuffers[nextIndex].Map(0, glyphletInfoBuffers[nextIndex].GetSize());
    }
    auto& glyph = glyphMap[id];
    
    for (size_t i = 0; i < glyph.text.Length(); i++)
    {
        
    }
}

void Glyphs::DrawMesh(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    if (glyphMap.empty())
        return;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 
        0, 1, &glyphletDescriptor->GetSets()[currentBufferIndex], 
        0, nullptr);
    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Glyphs::buildGlyphlets()
{
    auto chs = Resource::ResourceManager::GetCurrent()->Characters;
    glyphlets.resize(chs.size());
    uint32_t totalSize = 0;
    for (size_t i = 0; i < chs.size(); i++)
    {
        auto& ch = chs[i];
        auto& glyphlet = glyphlets[i];
        std::unordered_map<uint32_t, uint8_t> vertexMap{};
        for (auto& index : ch.indices)
        {
            if (vertexMap.try_emplace(index, static_cast<uint8_t>(vertexMap.size())).second)
                glyphlet.vertices.push_back(index);

            glyphlet.indices.push_back(vertexMap[index]);
        }
        totalSize += ch.indices.size() * sizeof(uint8_t) + ch.vertices.size() * sizeof(uint32_t) + 3;
    }
    glyphletBuffer = Buffer(aDevice, totalSize, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    glyphletBuffer.Map(0, glyphletBuffer.GetSize());
}