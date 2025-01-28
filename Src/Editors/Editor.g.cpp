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
	node0->ControlSize = {0.0f, 0.0f};
	node0->Spacing = {0.01f};
	node0->VerticalAlignment = {Stretch};
	node0->HorizontalAlignment = {Stretch};
	node0->Orientation = {Vertical};
	node0->Color = {0.156f, 0.156f, 0.156f};
	node0->Padding = {0.05f, 0.01f};
	TextBlock* node1 = new TextBlock();
	node1->Text("AnA");
	node1->Color = {0.8f, 0.8f, 0.8f};
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
	TextBlock* node6 = new TextBlock();
	node6->Text("Near");
	node0->Child(node6);
	Slider* node7 = new Slider();
	controlMap.insert(std::pair<std::string, Controls::Control*>("nearSlider", (Controls::Control*)node7));
	node7->HorizontalAlignment = {Stretch};
	node7->ControlSize = {0.0f, 0.02f};
	node7->Value = {0.05f / 32.0f};
	node0->Child(node7);
	TextBlock* node8 = new TextBlock();
	node8->Text("Far");
	node0->Child(node8);
	Slider* node9 = new Slider();
	controlMap.insert(std::pair<std::string, Controls::Control*>("farSlider", (Controls::Control*)node9));
	node9->HorizontalAlignment = {Stretch};
	node9->ControlSize = {0.0f, 0.02f};
	node9->Value = {1.0f};
	node0->Child(node9);
	Scene* node10 = new Scene();
	node10->HorizontalAlignment = {Stretch};
	node10->ImageInfo(Resource::ResourceManager::GetCurrent()->ShadowMap.GetDescriptorImageInfos()[0]);
	node10->ControlSize = {0.0f, 0.3f};
	node0->Child(node10);
	Button* node11 = new Button();
	node11->HorizontalAlignment = {Start};
	node11->PointerEvents[PointerEventType::Released].emplace_back(exitButton_Click);
	node0->Child(node11);
	TextBlock* node12 = new TextBlock();
	node12->Text("Exit");
	node11->Child(node12);

	return node0;
}
