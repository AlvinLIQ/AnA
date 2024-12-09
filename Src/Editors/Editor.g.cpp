#include "../GUI/Controls/Headers/Button.hpp"
#include "../GUI/Controls/Headers/Scene.hpp"
#include "../GUI/Controls/Headers/Slider.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"
#include "Headers/Editor.hpp"

using namespace AnA;
using namespace Controls;
using namespace Editors;

Controls::Control* Editor::InitControl()
{
StackPanel* node0 = new StackPanel();
node0->ControlSize = {300.0f, 0.0f};
node0->Spacing = {0.01f};
node0->VerticalAlignment = {Stretch};
node0->HorizontalAlignment = {Stretch};
node0->Orientation = {Vertical};
node0->Color = {0.8f, 0.8f, 0.8f};
TextBlock* node1 = new TextBlock();
node1->Text("AnA");
node0->Child(node1);
Button* node2 = new Button();
node2->HorizontalAlignment = {Start};
node2->PointerEvents[PointerEventType::Released].emplace_back(loadModelButton_Click);
node0->Child(node2);
TextBlock* node3 = new TextBlock();
node3->Text("Load Model");
node2->Child(node3);
Button* node4 = new Button();
node4->HorizontalAlignment = {Start};
node4->PointerEvents[PointerEventType::Released].emplace_back(saveSceneButton_Click);
node0->Child(node4);
TextBlock* node5 = new TextBlock();
node5->Text("Save Scene");
node4->Child(node5);
Slider* node6 = new Slider();
node6->HorizontalAlignment = {Stretch};
node6->ControlSize = {0.0f, 0.02f};
node0->Child(node6);
Scene* node7 = new Scene();
node7->HorizontalAlignment = {Stretch};
node7->ImageInfo(Resource::ResourceManager::GetCurrent()->ShadowMap.GetDescriptorImageInfos()[0]);
node7->ControlSize = {0.0f, 0.3f};
node0->Child(node7);

return node0;
}
