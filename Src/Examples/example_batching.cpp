#define ANA_INCLUDE_CONTROL
#include "../Core/Headers/App.hpp"

using namespace AnA;

class BatchedObjects
{
public:
    BatchedObjects()
    {
        
    }
    ~BatchedObjects()
    {
        for (auto& object : objects)
        {
            delete object;
        }
    }
    std::vector<Object*>& Get()
    {
        return objects;
    }
private:
    std::vector<Object*> objects;
    
};

BatchedObjects* objects = nullptr;
Texture* texture;
const VkDeviceSize offset = 0;

int main()
{
    App app{};
    app.Init();
    objects = new BatchedObjects();
    texture = new Texture((uint32_t)-1, app.GetDevice());
    app.Run([](VkCommandBuffer commandBuffer)
    {
        auto& shader = Resource::ResourceManager::GetCurrent()->Shaders[0];
        auto& sets = shader->GetDescriptorSets()[SwapChain::GetCurrent()->CurrentFrame];
        sets[DEFAULT_SAMPLER_LAYOUT] = texture->GetDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 
            0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
        uint32_t i = 0;
        for (auto& object : objects->Get())
        {
            auto& vertexBuffer = object->Model->GetVertexBuffer()->GetBuffer();
            vkCmdBindVertexBuffers(commandBuffer, i++, 1, &vertexBuffer, &offset);
            
        }
    });
    delete objects;
    delete texture;
    return 0;
}