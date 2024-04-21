#include "Headers/Shape.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Shapes::Shapes(Device& mDeivce) : aDevice {mDeivce}
{
    auto& descriptorSetLayout = Resource::ResourceManager::GetCurrent()->Shaders[1].GetDescriptors()[DEFAULT_SSBO_LAYOUT]->GetLayout();
    ssboDescriptor = new Descriptor(aDevice, 1, 
        MaxBatchSize,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT);
    shapeBuffer = new Buffer(aDevice, sizeof(Shape) * MaxBatchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    shapeBuffer->Map(0, shapeBuffer->GetSize());
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shapeBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = shapeBuffer->GetSize();
    ssboDescriptor->UpdateDescriptorSets(&bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

Shapes::~Shapes()
{
    delete ssboDescriptor;
    delete shapeBuffer;
}

void Shapes::PrepareDraw(Controls::Control* control)
{
    uint32_t newShapeCount;
    control->PrepareDraw((Shape*)shapeBuffer->GetMappedData(), newShapeCount);
    shapeCount = newShapeCount;
}

void Shapes::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
    pipelineLayout, 0, 1, 
    &ssboDescriptor->GetSets()[0], 0, nullptr);
    vkCmdDraw(commandBuffer, shapeCount * 6, 1, 0, 0);
}

Shape* Shapes::GetBufferData()
{
    return (Shape*)shapeBuffer->GetMappedData();
}