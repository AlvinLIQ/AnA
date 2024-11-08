#include "Headers/Editor.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"
#include "../GUI/Controls/Headers/Button.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"

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

    StackPanel* node0 = new StackPanel();
    node0->ControlSize = {300.0f, 0.0f};
    node0->Spacing = {0.01f};
    node0->VerticalAlignment = {Stretch};
    node0->HorizontalAlignment = {Stretch};
    node0->Orientation = {Vertical};
    node0->Color = {0.92f, 0.92f, 0.92f};
    Button* node1 = new Button();
    node1->HorizontalAlignment = {Start};
    node1->PointerEvents[PointerEventType::Released].emplace_back(loadModelButton_Click);
    node0->Child(node1);
    TextBlock* node2 = new TextBlock();
    node2->Text("Load Model");
    node1->Child(node2);
    Button* node3 = new Button();
    node3->HorizontalAlignment = {Start};
    node0->Child(node3);
    TextBlock* node4 = new TextBlock();
    node4->Text("Save Scene");
    node3->Child(node4);
    
    aResourceManager->MainControl = node0;
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
    char path[256] = "";
    switch (glfwGetPlatform())
    {
#ifdef _WIN32
        case GLFW_PLATFORM_WIN32:
        
        break;
#else
        case GLFW_PLATFORM_X11:
        case GLFW_PLATFORM_WAYLAND:
            FILE* f = popen("/usr/bin/zenity --file-selection", "r");
            fgets(path, 256, f);
            size_t len = strlen(path);
            if (len <= 1)
                return;
            path[len- 1] = '\0';
            pclose(f);
            MeshInfo mesh;
            memcpy(mesh.filePath, path, 256);
            mesh.tetureId = 0;
            Resource::ResourceManager::GetCurrent()->SceneObjects.Append(std::vector<MeshInfo>(1, mesh));
            break;
#endif
    };
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
    editor.Run();
    return 0;
}