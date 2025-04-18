#include "Headers/Shape.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Shapes::Shapes(Device* mDeivce) : aDevice{mDeivce}
{
    imageInfos.resize(MaxBatchSize);
    auto& samplerLayout = Resource::ResourceManager::GetCurrent()->Shaders[1].GetDescriptors()[1]->GetLayout();
    samplersDescriptor = new Descriptor(aDevice, 1, 
        MaxBatchSize,
        samplerLayout,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        );
    auto& ssboLayout = Resource::ResourceManager::GetCurrent()->Shaders[1].GetDescriptors()[DEFAULT_VERTEX_LAYOUT]->GetLayout();
    ssboDescriptor = new Descriptor(aDevice, 1, 
        1,
        ssboLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        );
    shapeBuffer = Buffer(aDevice, sizeof(Shape) * MaxBatchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    shapeBuffer.Map(0, shapeBuffer.GetSize());
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shapeBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = shapeBuffer.GetSize();
    ssboDescriptor->UpdateDescriptorSets(&bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    indirectBuffer = Buffer(aDevice, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    indirectBuffer.Map(0, indirectBuffer.GetSize());
    auto indirectCommand = ((VkDrawIndirectCommand*)indirectBuffer.GetMappedData());
    indirectCommand->firstInstance = 0;
    indirectCommand->firstVertex = 0;
    indirectCommand->instanceCount = 1;
    
    countBuffer = Buffer(aDevice, 4, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    countBuffer.Map(0, 4);
    *((uint32_t*)countBuffer.GetMappedData()) = 1;
}

Shapes::~Shapes()
{
    delete samplersDescriptor;
    delete ssboDescriptor;
}

void Shapes::PrepareDraw(Controls::Control* control)
{
    uint32_t newShapeCount = 0;
    control->PrepareDraw((Shape*)shapeBuffer.GetMappedData(), imageInfos, newShapeCount);
    ((VkDrawIndirectCommand*)indirectBuffer.GetMappedData())->vertexCount = (shapeCount = newShapeCount) * 6;
    samplersDescriptor->UpdateDescriptorSets(imageInfos.data(), newShapeCount, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    sets[0] = ssboDescriptor->GetSets()[0];
    sets[1] = samplersDescriptor->GetSets()[0];
}

void Shapes::Bind(CommandBuffer& commandBuffer, uint32_t)
{
    auto& shader = Resource::ResourceManager::GetCurrent()->Shaders[1];
    shader.GetPipeline()->Bind(commandBuffer);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        shader.GetPipelineLayout(), 0, numsof(sets), 
        sets, 0, nullptr);
}

void Shapes::Draw(CommandBuffer& commandBuffer)
{
    vkCmdDraw(commandBuffer, shapeCount * 6, 1, 0, 0);
}

void Shapes::DrawIndirect(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndirectCount(commandBuffer, indirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndirectCommand));
}
