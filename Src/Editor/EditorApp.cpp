#include "Headers/EditorApp.hpp"
#include "Headers/FileDialog.hpp"
#include "../GUI/Controls/Headers/ToggleSwitch.hpp"
#include "../GUI/Controls/Headers/Slider.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"

using namespace AnA;
using namespace Editor;
using namespace Controls;

EditorApp::EditorApp() : App()
{

}

EditorApp::~EditorApp()
{
}

void EditorApp::Init()
{
    App::Init();
    aResourceManager.MainControl = InitControl();
    sceneOffset.x = EDITOR_LEFT_PANEL_WIDTH;
    aInputManager.GlobalProfile.flag = Input::InputProfileFlags::None;
    Input::KeyMapConfig keyMapConfig;
    keyMapConfig.keyCode = GLFW_KEY_TAB;
    keyMapConfig.param = &aInputManager;
    keyMapConfig.callBack = [](void* param)
    {
        auto aInputManager = reinterpret_cast<Input::InputManager*>(param);
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
    aInputManager.GlobalProfile.keyMapConfigs.push_back(keyMapConfig);
    Controls::Control::GetInputProfile(aResourceManager.MainControl, aInputManager.GetProfiles());
}

void EditorApp::onLoop()
{
    aResourceManager.LockCamera = static_cast<Controls::ToggleSwitch*>(controlMap["camLockToggle"])->Toggled;
    aResourceManager.MainCameraInfo.near = 0.05f + static_cast<Controls::Slider*>(controlMap["nearSlider"])->Value * (32.0f - 0.05f);
    aResourceManager.MainCameraInfo.far = static_cast<Controls::Slider*>(controlMap["farSlider"])->Value * 32.0f;
    aResourceManager.MainCameraInfo.UpdateCameraPerspective(aResourceManager.MainCamera);
}

void EditorApp::loadModelButton_Click(void* , PointerEventArgs& )
{
    FileDialog fileDialog{};
    auto path = fileDialog.Run();
    if (path.empty())
        return;
    MeshInfo mesh;
    memcpy(mesh.filePath, path.c_str(), path.length() + 1);
    mesh.tetureId = 0;
    Resource::ResourceManager::GetCurrent()->MainScene.Append(std::vector<MeshInfo>(1, mesh));
    auto panel = 
            static_cast<Controls::StackPanel*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["modelList"]);
    panel->Child(static_cast<Controls::Control*>(new Controls::TextBlock(path.c_str(), 
                                {0.8f, 0.8f, 0.8f, 1.0f})));
}

void EditorApp::saveSceneButton_Click(void* , PointerEventArgs& )
{
    FileDialog fileDialog{"--save"};
    auto path = fileDialog.Run();
    if(path.empty())
        return;
    /*
    FILE* f = fopen(path.c_str(), "wb");
    auto objects = Resource::ResourceManager::GetCurrent()->MainScene;
    fwrite((const void*)objects.Get(), sizeof(Mesh), objects.GetMeshCount(), f);

    fclose(f);*/
}

void EditorApp::exitButton_Click(void* , PointerEventArgs& )
{
    exit(0);
}

//const VkDeviceSize offset = 0;

int main()
{
    EditorApp editor{};
    editor.Init();
    auto scene = ReadFile("Scenes/scene.ana");
    auto& meshes = Resource::ResourceManager::GetCurrent()->MainScene;
    meshes.EnableUpdate = true;
    meshes.Append(reinterpret_cast<MeshInfo*>(scene.data()), scene.size() / sizeof(MeshInfo));
    editor.Run();
    return 0;
}