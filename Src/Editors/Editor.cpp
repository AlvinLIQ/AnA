#include "Headers/Editor.hpp"
#include "Headers/FileDialog.hpp"

using namespace AnA;
using namespace Editors;
using namespace Controls;

#define EDITOR_LEFT_PANEL_WIDTH 300

Editor::Editor() : App()
{

}

Editor::~Editor()
{

}

void Editor::Init()
{
    App::Init();
    aResourceManager->MainControl = InitControl();
    sceneOffset.x = EDITOR_LEFT_PANEL_WIDTH;
    aInputManager->GlobalProfile.flag = Input::InputProfileFlags::None;
    Input::KeyMapConfig keyMapConfig;
    keyMapConfig.keyCode = GLFW_KEY_TAB;
    keyMapConfig.param = aInputManager;
    keyMapConfig.callBack = [](void* param)
    {
        auto aInputManager = (Input::InputManager*)param;
        /*
        auto& activeProfile = aInputManager->GetActiveProfile();
        aInputManager->ProcessProfileFlag((activeProfile.flag ^= Input::Disabled) & Input::Disabled ? 
            aInputManager->GlobalProfile.flag :
            activeProfile.flag);*/
        if (&aInputManager->GetActiveProfile() == aInputManager->GetProfiles().data())
        {
            aInputManager->SetActiveProfile(1);
        }
        else
        {
            aInputManager->SetActiveProfile(0);
        }
    };
    aInputManager->GlobalProfile.keyMapConfigs.push_back(keyMapConfig);
    Controls::Control::GetInputProfile(aResourceManager->MainControl, aInputManager->GetProfiles());
}

void Editor::loadModelButton_Click(void* control, PointerEventArgs& args)
{
    FileDialog fileDialog{};
    auto path = fileDialog.Run();
    MeshInfo mesh;
    memcpy(mesh.filePath, path.c_str(), path.length());
    mesh.tetureId = 0;
    Resource::ResourceManager::GetCurrent()->SceneObjects.Append(std::vector<MeshInfo>(1, mesh));
}

void Editor::saveSceneButton_Click(void* control, PointerEventArgs& args)
{
    FileDialog fileDialog{"--save"};
    auto path = fileDialog.Run();
    if(path.empty())
        return;
    FILE* f = fopen(path.c_str(), "wb");
    auto objects = Resource::ResourceManager::GetCurrent()->SceneObjects;
    fwrite((const void*)objects.Get(), sizeof(Mesh), objects.GetMeshCount(), f);

    fclose(f);
}

const VkDeviceSize offset = 0;

int main()
{
    Editor editor{};
    editor.Init();
    auto scene = ReadFile("Scenes/scene.ana");
    auto& meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    //std::vector<std::string> files(1000, "Models/cube.obj");
    //for (int i = 0; i < 1000; i++)
    //    meshInfos.push_back({"Models/cube.obj", {{random_double(), random_double(), random_double()}, {random_double(), random_double(), random_double()}}});
    meshes.Append((MeshInfo*)scene.data(), scene.size() / sizeof(MeshInfo));
    //MeshInfo meshInfo = {"Models/cube.obj", {{}, {1.0f, 1.0f, 0.3f}}, 4};
    //meshes.Append(&meshInfo, 1);
    editor.Run();
    return 0;
}