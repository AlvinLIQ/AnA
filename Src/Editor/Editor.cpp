#include "Headers/Editor.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"

using namespace AnA;
using namespace Editors;

Editor::Editor() : App()
{

}

Editor::~Editor()
{

}

void Editor::Init()
{
    App::Init();
    aResourceManager->MainControl = new Controls::StackPanel();
}

void Editor::onCommandBufferRecording(VkCommandBuffer& commandBuffer)
{
    auto extent = GetSwapChain().GetExtent();
    extent.width = sceneOffset.x;
    //Update GUI Command Buffers here
    App::onCommandBufferRecording(commandBuffer);
}


#define INCLUDE_STB_IMAGE


using namespace AnA;
//using namespace AnA::Controls;

const VkDeviceSize offset = 0;
std::vector<MeshInfo> meshInfos = 
{
    {"Models/cube.obj", {{3.0, 0.5, 0.0}, {11.4f, 0.02f, 11.4}}, 0},
    {"Models/torus.obj", {{0.f, 0.f , 1.5f}, {.7f, .7f, .7f}}, 1},
    {"Models/cube.obj", {{-1.5, -.5, 1.5}, {0.7f, 0.7f, 0.7f}}, 2},
    {"Models/cube.obj", {{1.5, -0.4, 0.0}, {0.4f, 0.4f, 0.4}}, 3}
};

int main()
{
    Editor editor{};
    editor.Init();
    auto& meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    //std::vector<std::string> files(1000, "Models/cube.obj");
    //for (int i = 0; i < 1000; i++)
    //    meshInfos.push_back({"Models/cube.obj", {{drand48(), drand48(), drand48()}, {drand48(), drand48(), drand48()}}});
    meshes.Append(meshInfos);
    editor.Run();
    return 0;
}