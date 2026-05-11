#include "Headers/ItemsPresenter.hpp"

using namespace AnA;
using namespace Controls;

ItemsPresenter::ItemsPresenter() : Control()
{

}

ItemsPresenter::~ItemsPresenter()
{
    for (auto item : items)
    {
        delete item;
    }
    items.clear();
}

void ItemsPresenter::Child(Control* newItem)
{
    items.push_back(newItem);
    newItem->Parent = this;
    RequestUpdate();
}

void ItemsPresenter::RemoveChild(Control* targetItem)
{
    if (targetItem == NULL)
        return;
    for (auto item = items.begin(); item < items.end(); item++)
    {
        if (*item == targetItem)
        {
            items.erase(item);
            delete targetItem;
            break;
        }
    }
    RequestUpdate();
}

void ItemsPresenter::RemoveChildAt(size_t index)
{
    if (index >= items.size())
        return;
    Control* targetItem = items[index];
    items.erase(items.begin() + index);
    delete targetItem;
    RequestUpdate();
}

void ItemsPresenter::PointerEventTrigger(PointerEventArgs& args)
{
    if (GetFocused() && !IsFocused())
    {
        GetFocused()->PointerEventTrigger(args);
        return;
    }
    for (auto& item : items)
    {
        if (args.Handled)
            return;
        item->PointerEventTrigger(args);
    }
    Control::PointerEventTrigger(args);
}
