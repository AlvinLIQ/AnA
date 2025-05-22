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
    loopCallback = EditorApp::onLoop;

    App::Init();
}

void EditorApp::onLoop()
{
    auto& aResourceManager = *Resource::ResourceManager::GetCurrent();
    auto editorApp = static_cast<EditorApp*>(App::GetCurrent());
    aResourceManager.LockCamera = static_cast<Controls::ToggleSwitch*>(editorApp->controlMap["camLockToggle"])->Toggle();
    aResourceManager.MainCameraInfo.near = 0.05f + static_cast<Controls::Slider*>(editorApp->controlMap["nearSlider"])->Value * (32.0f - 0.05f);
    aResourceManager.MainCameraInfo.far = static_cast<Controls::Slider*>(editorApp->controlMap["farSlider"])->Value * 32.0f;
    aResourceManager.MainCameraInfo.UpdateCameraPerspective(aResourceManager.MainCamera);

    (editorApp->controlMap["shadowMapView"])->TextureLayer =
        uint32_t(std::min(2.0f, static_cast<Controls::Slider*>(editorApp->controlMap["shadowMapSlider"])->Value * 3.0f));
    aResourceManager.GlobalLight.Direction = {static_cast<Controls::Slider*>(editorApp->controlMap["lightX"])->Value * 10.0f - 5.0f,
    static_cast<Controls::Slider*>(editorApp->controlMap["lightY"])->Value * 10.0f - 5.0f,
    static_cast<Controls::Slider*>(editorApp->controlMap["lightZ"])->Value * 10.0f - 5.0f};
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
    /* Fix this later
    FileDialog fileDialog{"--save"};
    auto path = fileDialog.Run();
    if(path.empty())
        return;
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    FILE* f = fopen(path.c_str(), "wb");
    auto& scene = resourceManager->MainScene;
    auto meshCount = scene.GetMeshCount();
    auto meshes = scene.Get();
    for (size_t i = 0; i < meshCount; i++)
    {
        auto& mesh = meshes[i];
        MeshInfo info{};
        memcpy(info.filePath, mesh.model->Path.data(), mesh.model->Path.size());
        info.tetureId = mesh.textureId;
        info.transform = mesh.transform;
        fwrite((const void*)&info, sizeof(MeshInfo), 1, f);
    }

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
    auto& scene = Resource::ResourceManager::GetCurrent()->MainScene;
    
    auto sceneFile = ReadFile("Scenes/scene.ana");
    scene.Append(reinterpret_cast<MeshInfo*>(sceneFile.data()), sceneFile.size() / sizeof(MeshInfo));
    auto& ch = Resource::ResourceManager::GetCurrent()->Characters['9'];
    std::vector<Model::Vertex> vertices{};
    for (auto& vertex : ch.vertices)
    {
        vertices.push_back({glm::vec3(vertex.x, -vertex.y, 0.0f), {0.0f, 0.0f, 0.0f}, {}});
    }
    scene.Append(vertices, ch.indices, {{0.0f, -2.0f, 0.0f}, glm::vec3{4.0f}}, 3);
    editor.Run();
    return 0;
}
