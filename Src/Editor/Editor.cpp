#include "Headers/Editor.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"
#include "../GUI/Controls/Headers/Button.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"

using namespace AnA;
using namespace Editors;

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

    auto panel = new Controls::StackPanel();
    panel->VerticalAlignment = AlignmentType::Center;
    panel->HorizontalAlignment = AlignmentType::Center;
    panel->Orientation = Controls::Orientations::Vertical;
    panel->Color = glm::vec3{0.92f};

    auto button = new Controls::Button();
    button->HorizontalAlignment = AlignmentType::Start;
    auto content = new Controls::TextBlock();
    content->Text("Load");
    button->Child(content);
    panel->Child(button);
    button->PointerEvents[PointerEventType::Released].push_back([](void* control, PointerEventArgs& args)
    {
        
    });

    button = new Controls::Button();
    button->HorizontalAlignment = AlignmentType::Center;
    panel->Child(button);

    button = new Controls::Button();
    button->HorizontalAlignment = AlignmentType::End;
    panel->Child(button);

    button = new Controls::Button();
    button->HorizontalAlignment = AlignmentType::Stretch;
    panel->Child(button);

    auto textBlock = new Controls::TextBlock();
    textBlock->Text("Test Message");
    textBlock->HorizontalAlignment = AlignmentType::Center;
    panel->Child(textBlock);
    
    aResourceManager->MainControl = panel;
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