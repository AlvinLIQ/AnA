#include "Headers/ItemPresenter.hpp"
#include "Headers/Types.hpp"
#include <vulkan/vulkan_core.h>

using namespace AnA;
using namespace Controls;

ItemPresenter::ItemPresenter() : Control()
{

}

ItemPresenter::~ItemPresenter()
{
    if (item != nullptr)
    {
        delete item;
        item = nullptr;
    }
}

void ItemPresenter::Child(Control* newItem)
{
    if (item != nullptr)
        delete item;

    item = newItem;
    item->Parent = this;
    RequestUpdate();
}

Control* ItemPresenter::Child()
{
    return item;
}

void ItemPresenter::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    GetSizeForRender();
    GetActualControlOffset();
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void ItemPresenter::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    if (item != nullptr)
    {
        Vec2 itemRenderOffset, itemRenderSize;
        GetItemRenderInfo(itemOffset, itemRenderOffset, itemRenderSize);

        item->RenderOffset(itemRenderOffset);
        item->RenderSize(itemRenderSize);
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void ItemPresenter::PointerEventTrigger(PointerEventArgs& args)
{
    Control::PointerEventTrigger(args);
    if (IsCursorInside() && item != nullptr)
        item->PointerEventTrigger(args);
}

void ItemPresenter::GetItemRenderInfo(const Vec2 itemOffset, Vec2& itemRenderOffset, Vec2& itemRenderSize)
{
    item->Extent = Extent;
    item->Aspect = Aspect;
    itemRenderSize = item->GetSizeForRender();
    itemRenderOffset = renderOffset;
    AlignmentType alignments[] = {HorizontalContentAlignment, VerticalContentAlignment};
    for (int i = 0; i < 2; i++)
    {
        if (renderSize[i] < itemRenderSize[i])
        {
            //renderOffset[i] = renderOffset[i] - renderSize[i] + itemRenderSize[i];
            renderSize[i] = std::min(itemRenderSize[i], maxSize[i]);
        }
        else if (alignments[i] == AlignmentType::Center)
        {
            itemRenderOffset[i] = renderOffset[i] + (renderSize[i] - itemRenderSize[i]) * 0.5f;
        }
        else if (alignments[i] == AlignmentType::End)
        {
            itemRenderOffset[i] = renderOffset[i] + renderSize[i] - itemRenderSize[i];
        }
        else
        {
            itemRenderOffset[i] = renderOffset[i];
            if(alignments[i] == AlignmentType::Stretch)
            {
                itemRenderSize[i] = renderSize[i];
            }
        }
    }
    itemRenderOffset += itemOffset;
}
