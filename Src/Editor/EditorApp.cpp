#include "Headers/EditorApp.hpp"
#include "Headers/FileDialog.hpp"
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
    keyMapConfig.keyCode = SDL_SCANCODE_TAB;
    keyMapConfig.param = &aInputManager;
    keyMapConfig.callback = [](void* param)
    {
        auto aInputManager = reinterpret_cast<Input::InputManager*>(param);
        /*
        auto& activeProfile = aInputManager->GetActiveProfile();
        aInputManager->ProcessProfileFlag((activeProfile.flag ^= Input::Disabled) & Input::Disabled ?
            aInputManager->GlobalProfile.flag :
            activeProfile.flag);*/
        auto editorApp = reinterpret_cast<EditorApp*>(EditorApp::GetCurrent());
        editorApp->ActionMode = ActionModes::Normal;
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
        createActionModeCallback<ActionModes::Grab>()
        , SDL_SCANCODE_G, ANA_PRESS});// Grab Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        createActionModeCallback<ActionModes::Scale>()
        , SDL_SCANCODE_S, ANA_PRESS});// Scale Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        createActionModeCallback<ActionModes::Rotate>()
        , SDL_SCANCODE_R, ANA_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
            createAxisTypeCallback<AxisType::X>()
            , SDL_SCANCODE_X, ANA_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
            createAxisTypeCallback<AxisType::Y>()
            , SDL_SCANCODE_Y, ANA_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
            createAxisTypeCallback<AxisType::Z>()
            , SDL_SCANCODE_Z, ANA_PRESS});// Rotate Mode
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            auto keys = SDL_GetKeyboardState(nullptr);
            if (editorApp->SelectedObjectData == nullptr || !keys[SDL_SCANCODE_LSHIFT])
                return;
            editorApp->ActionMode = ActionModes::Normal;

            auto modelList = reinterpret_cast<Controls::ObjectView*>(editorApp->controlMap["modelList"]);
            Resources::ResourceManager::GetCurrent()->MainScene.RemoveAt(editorApp->SelectedObjectData->id);
            int selectionIndex = modelList->SelectionIndex();
            modelList->RemoveChildAt(selectionIndex);
            for (int i = selectionIndex; i < int(modelList->Children().size()); i++)
                reinterpret_cast<ObjectViewItem*>(modelList->Children()[i])->Data.id--;
        }
        , SDL_SCANCODE_D, ANA_PRESS});
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            if (editorApp->ActionMode == ActionModes::Normal || editorApp->SelectedObjectData == nullptr)
                return;

            editorApp->ActionMode = ActionModes::Normal;
            auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
            object.transform = _originalTransform;
            editorApp->aResourceManager.MainScene.UpdateMeshTransform(editorApp->SelectedObjectData->id);
        }
        , SDL_SCANCODE_ESCAPE, ANA_PRESS});
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            auto transform = editorApp->aResourceManager.MainCamera.CameraTransform;
            printf("{{%f, %f, %f},\n{1.0f, 1.0f, 1.0f},\n{%f, %f, %f}}\n", transform.translation.x, transform.translation.y, transform.translation.z,
                transform.rotation.x, transform.rotation.y, transform.rotation.z);
        }
            , SDL_SCANCODE_F, ANA_PRESS});// Output Camera Transform
    editorInputProfile.opKeyMapConfigs.push_back({this,
        [](void* param)
        {
            auto editorApp = reinterpret_cast<EditorApp*>(param);
            if (editorApp->SelectedObjectData == nullptr)
                return;

            auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
            auto transform = object.transform;
            printf("{{%f, %f, %f},\n{1.0f, 1.0f, 1.0f},\n{%f, %f, %f}}\n", transform.translation.x, transform.translation.y, transform.translation.z,
                transform.rotation.x, transform.rotation.y, transform.rotation.z);
        }
            , SDL_SCANCODE_O, ANA_PRESS});// Output Object Transform
    editorInputProfile.cursorConfigs.push_back({this,
    [](void* param, Input::CursorArgs& curArgs, int leftButtonAction)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(param);
        if (editorApp->ActionMode == ActionModes::Normal || editorApp->SelectedObjectData == nullptr)
            return;

        if (leftButtonAction)
        {
            editorApp->ActionMode = ActionModes::Normal;
            return;
        }
        auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
        auto duration = curArgs.duration;
        auto& mainCamera = editorApp->aResourceManager.MainCamera;
        float speedRatio = mainCamera.GetSpeedRatio();
        switch(editorApp->ActionMode)
        {
        case ActionModes::Grab:
            if (editorApp->FocusedAxis == AxisType::All)
            {
                object.transform.translation -=
                    glm::vec3(cosf(mainCamera.CameraTransform.rotation.y),
                        0.0f,
                        -sinf(mainCamera.CameraTransform.rotation.y)) * duration.x.As<float>() * speedRatio * 10000.0f;
                object.transform.translation.y -= duration.y.As<float>() * speedRatio * 10000.0f;
            }
            else
            {
                object.transform.translation[(int)editorApp->FocusedAxis] += (duration.x - duration.y).As<float>() * speedRatio * 10000.0f;
            }
            break;
        case ActionModes::Scale:
        {
            float d = (duration.x - duration.y).As<float>() * speedRatio * 10000.0f;
            if (editorApp->FocusedAxis == AxisType::All)
                object.transform.scale += d;
            else
                object.transform.scale[(int)editorApp->FocusedAxis] += d;
            break;
        }
        case ActionModes::Rotate:
        {
            const float rotateSpeed = speedRatio * mainCamera.GetRotateSpeed() * 6.283f * 80.f;
            if (editorApp->FocusedAxis == AxisType::All)
            {
                object.transform.rotation.y =
                    glm::mod(object.transform.rotation.y - static_cast<float>(duration.x) * rotateSpeed,
                        glm::two_pi<float>());
                object.transform.rotation.x += static_cast<float>(duration.y) * rotateSpeed;
            }
            else
            {
                object.transform.rotation[(int)editorApp->FocusedAxis] += (duration.x.value - duration.y) * rotateSpeed;
            }
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
    static_cast<Controls::ListView*>((static_cast<EditorApp*>(App::GetCurrent())->controlMap["camLockToggle"]))->Select(0);

    auto pageTabs = static_cast<Controls::ListView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["pageTabs"]);
    pageTabs->Select(0);
    loopCallback = EditorApp::onLoop;

    App::Init();
}

void EditorApp::onLoop()
{
    auto& aResourceManager = *Resources::ResourceManager::GetCurrent();
    auto editorApp = static_cast<EditorApp*>(App::GetCurrent());
    aResourceManager.LockCamera = static_cast<Controls::ListView*>(editorApp->controlMap["camLockToggle"])->SelectionIndex() == 1;
    aResourceManager.MainCameraInfo.near = 0.05f + static_cast<Controls::Slider*>(editorApp->controlMap["nearSlider"])->Value() * (1000.0f - 0.05f);
    aResourceManager.MainCameraInfo.far = static_cast<Controls::Slider*>(editorApp->controlMap["farSlider"])->Value() * 1000.0f;
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
        mesh.textureId = 0;
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
        info.textureId = mesh.textureId;
        info.transform = mesh.transform;
        fwrite((const void*)&info, sizeof(MeshInfo), 1, f);
    }

    fclose(f);
}

void EditorApp::exitButton_Click(void* , PointerEventArgs& )
{
    exit(0);
}

void EditorApp::pageTabs_SelectionChanged(void* )
{
    auto pageView = static_cast<Controls::PageView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["pageView"]);
    auto pageTabs = static_cast<Controls::ListView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["pageTabs"]);
    pageView->PageIndex(pageTabs->SelectionIndex());
}

void EditorApp::mainScene_MeshAppend(std::string name, uint32_t id)
{
    auto panel =
            static_cast<Controls::ObjectView*>(static_cast<EditorApp*>(App::GetCurrent())->controlMap["modelList"]);
    ObjectViewItemData itemData;
    itemData.id = id;
    itemData.name = name.substr(name.find_last_of('/') + 1).c_str();
    itemData.icon = "";
    panel->AddItem(itemData);
}
//const VkDeviceSize offset = 0;

template<ActionModes actionMode>
Input::RegularCallBack EditorApp::createActionModeCallback()
{
    return [](void* param)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(param);
        if (editorApp->SelectedObjectData == nullptr)
            return;
        editorApp->FocusedAxis = AxisType::All;

        auto& object = editorApp->aResourceManager.MainScene.At(editorApp->SelectedObjectData->id);
        _originalTransform = object.transform;
        editorApp->ActionMode = editorApp->ActionMode == actionMode ? ActionModes::Normal : actionMode;
    };
}

template<AxisType axisType>
Input::RegularCallBack EditorApp::createAxisTypeCallback()
{
    return [](void* param)
    {
        auto editorApp = reinterpret_cast<EditorApp*>(param);
        if (editorApp->SelectedObjectData == nullptr)
            return;

        editorApp->FocusedAxis = editorApp->FocusedAxis == axisType ? AxisType::All : axisType;
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
