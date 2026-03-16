#include "Headers/Shape.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Shapes::Shapes(Device* mDeivce) : aDevice{mDeivce}
{
    imageInfos.resize(MaxBatchSize);
    auto& samplerLayout = Resources::ResourceManager::GetCurrent()->Shaders[1].GetDescriptors()[1].GetLayout();
    samplersDescriptor = new Descriptor(aDevice, 1,
        MaxBatchSize,
        1,
        samplerLayout,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        );
    auto& ssboLayout = Resources::ResourceManager::GetCurrent()->Shaders[1].GetDescriptors()[0].GetLayout();
    ssboDescriptor = new Descriptor(aDevice, 1,
        1,
        1,
        ssboLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        );
    shapeBuffer = Buffer(aDevice, sizeof(Shape) * MaxBatchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    shapeBuffer.Map();
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shapeBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = shapeBuffer.GetSize();
    ssboDescriptor->UpdateDescriptorSets(&bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    indirectBuffer = Buffer(aDevice, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    indirectBuffer.Map();
    auto indirectCommand = ((VkDrawIndirectCommand*)indirectBuffer.GetMappedData());
    indirectCommand->firstInstance = 0;
    indirectCommand->firstVertex = 0;
    indirectCommand->instanceCount = 1;

    countBuffer = Buffer(aDevice, 4, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    countBuffer.Map();
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
    updated = true;
}

bool Shapes::NeedUpdate()
{
    return !updated;
}

void Shapes::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t)
{
    shader.GetPipeline().Bind(commandBuffer);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, numsof(sets),
        sets, 0, nullptr);
    vkCmdSetPrimitiveTopology(commandBuffer, Topology);
}

void Shapes::Draw(CommandBuffer& commandBuffer)
{
    vkCmdDraw(commandBuffer, shapeCount * 6, 1, 0, 0);
}

void Shapes::DrawIndirect(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndirectCount(commandBuffer, indirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndirectCommand));
}
