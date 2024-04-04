#define ANA_INCLUDE_CONTROL
#include "../Core/Headers/App.hpp"
#include "../Core/Resources/Headers/Mesh.hpp"

using namespace AnA;


Texture* texture;
const VkDeviceSize offset = 0;
std::vector<MeshInfo> meshInfos = 
{
    {"Models/cube.obj", {{3.0, 0.5, 0.0}, {11.4f, 0.02f, 11.4}}},
    {"Models/torus.obj", {{0.f, 0.f , 1.5f}, {.7f, .7f, .7f}}},
    {"Models/cube.obj", {{-1.5, -.5, 1.5}, {0.7f, 0.7f, 0.7f}}},
    {"Models/cube.obj", {{1.5, -0.4, 0.0}, {0.4f, 0.4f, 0.4}}}
};

int main()
{
    App app{};
    app.Init();
    auto meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    //std::vector<std::string> files(1000, "Models/cube.obj");
    for (int i = 0; i < 1000; i++)
        meshInfos.push_back({"Models/cube.obj", {{drand48(), drand48(), drand48()}, {drand48(), drand48(), drand48()}}});
    meshes->Append(meshInfos);
    texture = new Texture((uint32_t)0xFFFFFFFF, app.GetDevice());
    app.Run([](VkCommandBuffer commandBuffer)
    {
        auto& shader = Resource::ResourceManager::GetCurrent()->Shaders[0];
        shader->GetPipeline()->Bind(commandBuffer);
        auto& sets = shader->GetDescriptorSets()[SwapChain::GetCurrent()->CurrentFrame];
        sets[DEFAULT_SAMPLER_LAYOUT] = texture->GetDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 
            0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
        auto meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
        meshes->Bind(commandBuffer);
        meshes->Draw(commandBuffer);
    });
    delete texture;
    return 0;
}