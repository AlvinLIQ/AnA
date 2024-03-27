#include "Headers/Lights.hpp"
#include "../Headers/Buffer.hpp"
#include "../Camera/Headers/Camera.hpp"

using namespace AnA;
using namespace Lights;

Light::Light(Device& mDevice) : aDevice {mDevice}
{
    createBuffers();
}

Light::~Light()
{
    for (auto& buffer : buffers)
    {
        delete buffer;
    }
}

Buffer** Light::GetBuffers()
{
    return buffers.data();
}

void Light::UpdateBuffers(Cameras::Camera& lightCamera, int currentFrame)
{

    auto& lightBufferObject = *(LightBufferObject*)buffers[currentFrame]->GetMappedData();
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
        lightBuffer = new Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        lightBuffer->Map(0, bufferSize);
        LightBufferObject& lbo = *((LightBufferObject*)lightBuffer->GetMappedData());
        lbo.proj = glm::mat4{1.0f};
        lbo.view = glm::mat4{1.0f};
        lbo.direction = glm::normalize(Direction);
        lbo.color = glm::vec3(0.2);
        lbo.ambient = 0.037f;
        //printf("%p\n", lightBuffer->GetMappedData());
    }
}