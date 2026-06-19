#include "Headers/ObjectView.hpp"
#include "Headers/Types.hpp"

using namespace AnA;
using namespace Controls;

ObjectView::ObjectView()
{
    Color = {};
}

ObjectView::~ObjectView()
{
}

void ObjectView::AddItem(ObjectViewItemData itemData, uint32_t iconLayer)
{
    auto item = new ObjectViewItem(itemData, iconLayer);
    item->HorizontalAlignment = Stretch;
    item->VerticalAlignment = Start;
    Child(item);
}
