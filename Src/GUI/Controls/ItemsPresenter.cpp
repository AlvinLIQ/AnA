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

void ItemsPresenter::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    Control::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
    for (auto& item : items)
    {
        item->PrepareDraw(shapeBuffer, imageInfos, shapeCount);
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

void ItemsPresenter::RemoveChildAt(size_t index)
{
    if (index >= items.size())
        return;
    Control* targetItem = items[index];
    items.erase(items.begin() + index);
    delete targetItem;
}

void ItemsPresenter::PointerEventTrigger(PointerEventArgs& args)
{
    if (args.Handled)
        return;
    for (auto& item : items)
    {
        item->PointerEventTrigger(args);
        if (args.Handled)
            return;
    }
    Control::PointerEventTrigger(args);
}