#include "Headers/ListView.hpp"
#include "Headers/ItemsPresenter.hpp"
#include "Headers/Types.hpp"

using namespace AnA;
using namespace Controls;

void ListView::ListView_PointerMoving(ListView* control, PointerEventArgs& args)
{
    for (auto& item : control->Children())
    {
        if (item->IsInside(args.Position))
        {
            if (item != control->hoverItem && item != control->selectedItem)
            {
                if (control->hoverItem != nullptr && control->hoverItem != control->selectedItem)
                {
                    control->hoverItem->Color = {};
                }
                control->hoverItem = item;
                item->Color = {0.25f, 0.25f, 0.25f, 1.0f};
                item->RequestUpdate();
            }
            return;
        }
    }
    if (control->hoverItem != nullptr && control->hoverItem != control->selectedItem)
    {
        control->hoverItem->Color = {};
        control->hoverItem->RequestUpdate();
        control->hoverItem = nullptr;
    }
}


void ListView::ListView_PointerPressed(ListView* control, PointerEventArgs& )
{
    if (control->hoverItem == control->selectedItem)
        return;
    if (control->hoverItem != nullptr)
    {
        control->Select(control->hoverItem);
    }
}

void ListView::ListView_PointerExited(ListView* control, PointerEventArgs& )
{
    if (control->hoverItem != nullptr)
    {
        if (control->hoverItem != control->selectedItem)
            control->hoverItem->Color = {};
        control->hoverItem->RequestUpdate();
        control->hoverItem = nullptr;
    }
}

ListView::ListView()
{
    PointerEvents[PointerEventType::Moving].push_back(reinterpret_cast<PointerEventHandler>(ListView::ListView_PointerMoving));
    PointerEvents[PointerEventType::Pressed].push_back(reinterpret_cast<PointerEventHandler>(ListView::ListView_PointerPressed));
    PointerEvents[PointerEventType::Exited].push_back(reinterpret_cast<PointerEventHandler>(ListView::ListView_PointerExited));
}

void ListView::Select(int index)
{
    if (selectedItem != nullptr)
        selectedItem->Color = {};
    if (index < 0)
    {
        selectionIndex = -1;
        selectedItem = nullptr;
    }
    else
    {
        auto item = items[index];
        item->Color = {0.3f, 0.3f, 0.3f, 1.0f};
        item->RequestUpdate();
        selectedItem = item;
        selectionIndex = index;
    }
    if (SelectionChanged != nullptr)
        SelectionChanged(selectedItem);
}
void ListView::Select(Control* item)
{
    for (size_t i = 0; i < items.size(); i++)
        if (items[i] == item)
        {
            Select(int(i));
            break;
        }
}

void ListView::RemoveChildAt(size_t index)
{
    ItemsPresenter::RemoveChildAt(index);
    if (selectedItem != nullptr)
    {
        if (selectionIndex == 0 && selectionIndex < int(items.size()))
            Select(0);
        else if (selectionIndex >= int(index))
            Select(selectionIndex - 1);
    }
}

void ListView::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    StackPanel::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
}
