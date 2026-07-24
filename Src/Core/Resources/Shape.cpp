#include "Headers/Shape.hpp"
#include "Headers/Shader.hpp"
#include "../GUI/Controls/Headers/Control.hpp"

using namespace AnA;

Shapes::Shapes(Device* mDeivce) : aDevice{mDeivce}
{
    shapeBuffer = Buffer(aDevice, sizeof(Shape) * MaxBatchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    shapeBuffer.Map();

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
}

void Shapes::PrepareDraw(Controls::Control* control)
{
    std::lock_guard<std::mutex> lock(_mutex);
    uint32_t newShapeCount = 0;
    control->PrepareDraw((Shape*)shapeBuffer.GetMappedData(), newShapeCount);
    ((VkDrawIndirectCommand*)indirectBuffer.GetMappedData())->vertexCount = (shapeCount = newShapeCount) * 6;
    updated = true;
}

bool Shapes::NeedUpdate()
{
    return !updated;
}

void Shapes::Bind(CommandBuffer& commandBuffer, Shader& shader)
{
    shader.GetPipeline().Bind(commandBuffer);
    aDevice->vkCmdSetPolygonModeEXT(commandBuffer, PolygonMode);
    vkCmdSetPrimitiveTopology(commandBuffer, Topology);

    uint32_t bufferIndex = 0;
    VkDeviceSize offset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 1, &bufferIndex, &offset);

    shapePushConstant.shapePtr = shapeBuffer.GetAddress();
    shapePushConstant.resolution = {float(commandBuffer.Extent.width), float(commandBuffer.Extent.height)};
    vkCmdPushConstants(commandBuffer, shader.GetPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT |
        VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(shapePushConstant),
        &shapePushConstant);
}

void Shapes::Draw(CommandBuffer& commandBuffer)
{
    vkCmdDraw(commandBuffer, shapeCount * 6, 1, 0, 0);
}

void Shapes::DrawIndirect(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndirectCount(commandBuffer, indirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndirectCommand));
}
