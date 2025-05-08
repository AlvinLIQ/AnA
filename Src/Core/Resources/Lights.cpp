#include "Headers/Lights.hpp"
#include "../Headers/Buffer.hpp"
#include "../Camera/Headers/Camera.hpp"
//#include "Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Lights;

Light::Light(Device* mDevice) : aDevice{mDevice}
{
    createBuffers();
}

Light::~Light()
{

}

std::vector<Buffer>& Light::GetBuffers()
{
    return buffers;
}

void Light::UpdateBuffers(Cameras::Camera& lightCamera, uint32_t bufferIndex)
{
    auto& lightBufferObject = *static_cast<LightBufferObject*>(buffers[bufferIndex].GetMappedData());
    //auto cameraPosition = glm::mat3(Resource::ResourceManager::GetCurrent()->MainCamera.GetInverseView()) * glm::vec3(Resource::ResourceManager::GetCurrent()->MainCamera.GetView()[3]);
    //auto target = cameraPosition - glm::vec3(Direction.x, Direction.y, Direction.z);
    //lightCamera.SetViewTarget(cameraPosition, target);
    lightCamera.SetViewDirection({}, Direction);
    lightBufferObject = {.proj = lightCamera.GetProjectionMatrix(), .view = lightCamera.GetView(),
                    .direction = Direction, .color = Color, .ambient = Ambient};
}

void Light::createBuffers()
{
    VkDeviceSize bufferSize = sizeof(LightBufferObject);
    buffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &lightBuffer : buffers)
    {
        lightBuffer = Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        lightBuffer.Map();
        LightBufferObject& lbo = *static_cast<LightBufferObject*>(lightBuffer.GetMappedData());
        lbo.proj = glm::mat4{1.0f};
        lbo.view = glm::mat4{1.0f};
        lbo.direction = glm::normalize(Direction);
        lbo.color = Color;
        lbo.ambient = Ambient;
        //printf("%p\n", lightBuffer->GetMappedData());
    }
}