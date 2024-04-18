#include "Headers/ItemsPresenter.hpp"

using namespace AnA;
using namespace Controls;

ItemsPresenter::ItemsPresenter() : Control()
{

}

ItemsPresenter::~ItemsPresenter()
{
    for (auto item = items.begin(); item < items.end(); item++)
    {
        Control* pItem = *item;
        delete pItem;
    }
    items.clear();
}

void ItemsPresenter::Draw(VkCommandBuffer commandBuffer)
{
    prepareDraw();
    Control::Draw(commandBuffer);
    for (auto& item : items)
    {
        item->Draw(commandBuffer);
    }
}

void ItemsPresenter::Child(Control* newItem)
{
    items.push_back(newItem);
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
}

void ItemsPresenter::RemoveChildAt(int index)
{
    if (index >= items.size())
        return;
    Control* targetItem = items[index];
    items.erase(items.begin() + index);
    delete targetItem;
}