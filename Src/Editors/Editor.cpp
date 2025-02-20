#include "Headers/Editor.hpp"
#include "Headers/FileDialog.hpp"
#include "../GUI/Controls/Headers/Slider.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"

using namespace AnA;
using namespace Editors;
using namespace Controls;

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

void Editor::onLoop()
{
    aResourceManager->MainCameraInfo.near = ((Controls::Slider*)controlMap["nearSlider"])->Value * 32.0f;
    aResourceManager->MainCameraInfo.far = ((Controls::Slider*)controlMap["farSlider"])->Value * 32.0f;
    aResourceManager->MainCameraInfo.UpdateCameraPerspective(aResourceManager->MainCamera);
}

void Editor::loadModelButton_Click(void* control, PointerEventArgs& args)
{
    FileDialog fileDialog{};
    auto path = fileDialog.Run();
    if (path.empty())
        return;
    MeshInfo mesh;
    memcpy(mesh.filePath, path.c_str(), path.length() + 1);
    mesh.tetureId = 0;
    Resource::ResourceManager::GetCurrent()->SceneObjects.Append(std::vector<MeshInfo>(1, mesh));
    ((Controls::StackPanel*)((Editor*)App::GetCurrent())->controlMap["modelList"])->Child((Controls::Control*)new Controls::TextBlock(path.c_str(), {0.8f, 0.8f, 0.8f}));
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

void Editor::exitButton_Click(void* control, PointerEventArgs& args)
{
    exit(0);
}

const VkDeviceSize offset = 0;

int main()
{
    Editor editor{};
    editor.Init();
    auto scene = ReadFile("Scenes/scene.ana");
    auto& meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    meshes.Append((MeshInfo*)scene.data(), scene.size() / sizeof(MeshInfo));
    editor.Run();
    return 0;
}