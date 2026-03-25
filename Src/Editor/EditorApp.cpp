#include "Headers/EditorApp.hpp"
#include "Headers/FileDialog.hpp"
#include "../GUI/Controls/Headers/ToggleSwitch.hpp"
#include "../GUI/Controls/Headers/Slider.hpp"
#include "../GUI/Controls/Headers/PageView.hpp"
#include "../GUI/Controls/Headers/ObjectView.hpp"


using namespace AnA;
using namespace Editor;
using namespace Controls;

Transform _originalTransform;

EditorApp::EditorApp() : App()
{

}

EditorApp::~EditorApp()
{
}

void EditorApp::Init()
{
    aResourceManager.MainControl = InitControl();
    aResourceManager.MainScene.MeshAppend = EditorApp::mainScene_MeshAppend;
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
        auto editorApp = reinterpret_cast<EditorApp*>(EditorApp::GetCurrent());
        editorApp->ActionMode = 0;
        if (&aInputManager->GetActiveProfile() == aInputManager->GetProfiles().data())
        {
            aInputManager->GlobalProfile.flag = Input::InputProfileFlags::None;
            aInputManager->SetActiveProfile(1);
        }
        else
        {
            aInputManager->GlobalProfile.flag = Input::InputProfileFlags::CursorDisabled;
            aInputManager->SetActiveProfile(0);
        }
    };
    aInputManager.GlobalProfile.keyMapConfigs.push_back(keyMapConfig);
    Controls::Control::GetInputProfile(aResourceManager.MainControl, aInputManager.GetProfiles());
    auto& editorInputProfile = aInputManager.GetProfiles().back();
//Transform Action Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        createActionModeCallback<1>()
        , GLFW_KEY_G, GLFW_PRESS});// Grab Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        createActionModeCallback<2>()
        , GLFW_KEY_S, GLFW_PRESS});// Scale Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        createActionModeCallback<3>()
        , GLFW_KEY_R, GLFW_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            if (editorApp->SelectedObjectData == nullptr)
                return;
            editorApp->ActionMode = 0;

            auto modelList = reinterpret_cast<Controls::ObjectView*>(editorApp->controlMap["modelList"]);
            Resources::ResourceManager::GetCurrent()->MainScene.RemoveAt(editorApp->SelectedObjectData->id);
            int selectionIndex = modelList->SelectionIndex();
            modelList->RemoveChildAt(selectionIndex);
            for (int i = selectionIndex; i < int(modelList->Children().size()); i++)
                reinterpret_cast<ObjectViewItem*>(modelList->Children()[i])->Data.id--;
        }
        , GLFW_KEY_D, GLFW_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            if (editorApp->ActionMode == 0 || editorApp->SelectedObjectData == nullptr)
                return;

            editorApp->ActionMode = 0;
            auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
            object.transform = _originalTransform;
            editorApp->aResourceManager.MainScene.UpdateMeshTransform(editorApp->SelectedObjectData->id);
        }
        , GLFW_KEY_ESCAPE, GLFW_PRESS});// Rotate Mode

    editorInputProfile.cursorConfigs.push_back({this,
    [](void* param, Input::CursorArgs& curArgs, int leftButtonAction)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(param);
        if (editorApp->ActionMode == 0 || editorApp->SelectedObjectData == nullptr)
            return;

        if (leftButtonAction)
        {
            editorApp->ActionMode = 0;
            return;
        }
        auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
        auto duration = curArgs.duration;
        auto& mainCamera = editorApp->aResourceManager.MainCamera;
        float speedRatio = mainCamera.GetSpeedRatio();
        switch(editorApp->ActionMode)
        {
        case 1:
            object.transform.translation -=
                glm::vec3(cosf(mainCamera.CameraTransform.rotation.y),
                0.0f,
                -sinf(mainCamera.CameraTransform.rotation.y)) * duration.x.As<float>() * speedRatio * 10000.0f;
            object.transform.translation.y -= duration.y.As<float>() * speedRatio * 10000.0f;
            break;
        case 2:
            object.transform.scale += (duration.x - duration.y).As<float>() * speedRatio * 10000.0f;
            break;
        case 3:
        {
            const float rotateSpeed = speedRatio * mainCamera.GetRotateSpeed() * 6.283f * 80.f;
            object.transform.rotation.y =
                glm::mod(object.transform.rotation.y - static_cast<float>(duration.x) * rotateSpeed,
                glm::two_pi<float>());
            object.transform.rotation.x += static_cast<float>(duration.y) * rotateSpeed;
        }
            break;
        default:
            break;
        }
        editorApp->aResourceManager.MainScene.UpdateMeshTransform(editorApp->SelectedObjectData->id);
    }, 0});

    auto panel = static_cast<Controls::ObjectView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["modelList"]);
    panel->SelectionChanged = [](void* param)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(EditorApp::GetCurrent());
        if (param == nullptr)
            editorApp->SelectedObjectData = nullptr;
        else
        {
            auto selectedItem = reinterpret_cast<ObjectViewItem*>(param);
            editorApp->SelectedObjectData = &selectedItem->Data;
        }
    };
    loopCallback = EditorApp::onLoop;

    App::Init();
}

void EditorApp::onLoop()
{
    auto& aResourceManager = *Resources::ResourceManager::GetCurrent();
    auto editorApp = static_cast<EditorApp*>(App::GetCurrent());
    aResourceManager.LockCamera = static_cast<Controls::ToggleSwitch*>(editorApp->controlMap["camLockToggle"])->Toggle();
    aResourceManager.MainCameraInfo.near = 0.05f + static_cast<Controls::Slider*>(editorApp->controlMap["nearSlider"])->Value() * (32.0f - 0.05f);
    aResourceManager.MainCameraInfo.far = static_cast<Controls::Slider*>(editorApp->controlMap["farSlider"])->Value() * 32.0f;
    aResourceManager.MainCameraInfo.UpdateCameraPerspective(aResourceManager.MainCamera);

    aResourceManager.GlobalLight.Direction = {static_cast<Controls::Slider*>(editorApp->controlMap["lightX"])->Value() * 10.0f - 5.0f,
    static_cast<Controls::Slider*>(editorApp->controlMap["lightY"])->Value() * 10.0f - 5.0f,
    static_cast<Controls::Slider*>(editorApp->controlMap["lightZ"])->Value() * 10.0f - 5.0f};
}

void EditorApp::loadModelButton_Click(void* , PointerEventArgs& )
{
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    resourceManager->TaskPool.Enqueue([resourceManager]()
    {
        FileDialog fileDialog{};
        auto path = fileDialog.Run();
        if (path.empty())
            return;
        MeshInfo mesh;
        memcpy(mesh.filePath, path.c_str(), path.length() + 1);
        mesh.tetureId = 0;
        resourceManager->MainScene.Append(std::vector<MeshInfo>(1, mesh));
        resourceManager->AppendCallback([]()
        {
            Control::RequestUpdate();
        });

    });
}

void EditorApp::saveSceneButton_Click(void* , PointerEventArgs& )
{
    FileDialog fileDialog{"--save"};
    auto path = fileDialog.Run();
    if(path.empty())
        return;
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    FILE* f = fopen(path.c_str(), "wb");
    auto& scene = resourceManager->MainScene;
    auto meshCount = scene.GetMeshCount();
    auto meshes = scene.Get();
    for (size_t i = 0; i < meshCount; i++)
    {
        auto& mesh = meshes[i];
        MeshInfo info{};
        auto& model = resourceManager->Meshes.MeshMap[mesh.modelID];
        memcpy(info.filePath, model->Path.data(), model->Path.size());
        info.tetureId = mesh.textureId;
        info.transform = mesh.transform;
        fwrite((const void*)&info, sizeof(MeshInfo), 1, f);
    }

    fclose(f);
}

void EditorApp::exitButton_Click(void* , PointerEventArgs& )
{
    exit(0);
}

void EditorApp::pageButton_Click(void* , PointerEventArgs& )
{
    auto pageView = static_cast<Controls::PageView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["pageView"]);
    pageView->PageIndex(1 - pageView->PageIndex());
}

void EditorApp::mainScene_MeshAppend(std::string name, uint32_t id)
{
    auto panel =
            static_cast<Controls::ObjectView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["modelList"]);
    ObjectViewItemData itemData;
    itemData.id = id;
    itemData.name = name.substr(name.find_last_of('/') + 1).c_str();
    panel->AddItem(itemData);
}
//const VkDeviceSize offset = 0;

template<char actionMode>
Input::RegularCallBack EditorApp::createActionModeCallback()
{
    return [](void* param)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(param);
        if (editorApp->SelectedObjectData == nullptr)
            return;

        auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
        _originalTransform = object.transform;
        editorApp->ActionMode = editorApp->ActionMode == actionMode ? 0 : actionMode;
    };
}

int main()
{
    EditorApp editor{};
    editor.Init();
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    auto& scene = resourceManager->MainScene;

    auto sceneFile = ReadFile("Scenes/test.ana");
    scene.Append(reinterpret_cast<MeshInfo*>(sceneFile.data()), sceneFile.size() / sizeof(MeshInfo));
    /*
    std::vector<Model::Vertex> vertices;
    std::vector<Model::Index> indices;
    Model::CreateVerticesFromFile("/home/alvin/Downloads/data4102-33_.txt", vertices);
    Model::CreateTerrainFromVertices(vertices, indices, 256);

    scene.Append(vertices, indices, {{0.0f, -8.0f, 0.0f}, glm::vec3{3.0f}, {0.35 * 3.14159, 0.0f, 0.0f}});

    TextInfo textInfo;
    textInfo.offset = {0.0f, 0.0f};
    textInfo.size = 20.0f;
    textInfo.text = "AnA";
    textInfo.color = glm::vec3{0.3f};
    resourceManager->TextContext.Insert(textInfo, 30);*/
    editor.Run();
    return 0;
}
