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

void ObjectView::AddItem(ObjectViewItemData itemData)
{
    auto item = new ObjectViewItem(itemData);
    item->HorizontalAlignment = Stretch;
    item->HorizontalContentAlignment = Start;
    item->VerticalAlignment = Start;
    Child(item);
}
