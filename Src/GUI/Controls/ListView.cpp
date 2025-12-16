#include "Headers/ListView.hpp"
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
                item->Color = {0.4f, 0.4f, 0.4f, 1.0f};
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
    if (control->selectedItem != nullptr)
    {
        control->selectedItem->Color = {};
    }
    if (control->hoverItem != nullptr)
    {
        control->selectedItem = control->hoverItem;
        control->hoverItem->Color = {0.5f, 0.5f, 0.5f, 1.0f};
        control->hoverItem->RequestUpdate();
        control->selectedItem = control->hoverItem;
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

void ListView::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    StackPanel::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
}
