#include "../GUI/Controls/Headers/Button.hpp"
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

return node0;
}
