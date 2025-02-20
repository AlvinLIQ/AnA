#include "../GUI/Controls/Headers/Button.hpp"
#include "../GUI/Controls/Headers/ListView.hpp"
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
	ListView* node6 = new ListView();
	controlMap.insert(std::pair<std::string, Controls::Control*>("modelList", (Controls::Control*)node6));
	node6->Orientation = {Vertical};
	node6->HorizontalAlignment = {Stretch};
	node6->RenderMode(Passive);
	node6->Color = {0.2f, 0.2f, 0.2};
	node0->Child(node6);
	TextBlock* node7 = new TextBlock();
	node7->Text("Near");
	node0->Child(node7);
	Slider* node8 = new Slider();
	controlMap.insert(std::pair<std::string, Controls::Control*>("nearSlider", (Controls::Control*)node8));
	node8->HorizontalAlignment = {Stretch};
	node8->ControlSize = {0.0f, 0.02f};
	node8->Value = {0.05f / 32.0f};
	node0->Child(node8);
	TextBlock* node9 = new TextBlock();
	node9->Text("Far");
	node0->Child(node9);
	Slider* node10 = new Slider();
	controlMap.insert(std::pair<std::string, Controls::Control*>("farSlider", (Controls::Control*)node10));
	node10->HorizontalAlignment = {Stretch};
	node10->ControlSize = {0.0f, 0.02f};
	node10->Value = {1.0f};
	node0->Child(node10);
	Scene* node11 = new Scene();
	node11->HorizontalAlignment = {Stretch};
	node11->ImageInfo(Resource::ResourceManager::GetCurrent()->ShadowMap.GetDescriptorImageInfos()[0]);
	node11->ControlSize = {0.0f, 0.3f};
	node0->Child(node11);
	Button* node12 = new Button();
	node12->HorizontalAlignment = {Start};
	node12->PointerEvents[PointerEventType::Released].emplace_back(exitButton_Click);
	node0->Child(node12);
	TextBlock* node13 = new TextBlock();
	node13->Text("Exit");
	node12->Child(node13);

	return node0;
}
