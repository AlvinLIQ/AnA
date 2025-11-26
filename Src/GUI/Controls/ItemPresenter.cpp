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
}

Control* ItemPresenter::Child()
{
    return item;
}

void ItemPresenter::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    GetSizeForRender();
    GetActualControlOffset();
    ItemPresenter::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void ItemPresenter::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    if (item != nullptr)
    {
        item->Extent = Extent;
        item->Aspect = Aspect;
        auto itemRenderSize = item->GetSizeForRender();
        Vec2 itemRenderOffset = renderOffset;
        if (renderSize.x() < itemRenderSize.x())
        {
            renderOffset.x() = renderOffset.x() - renderSize.x() + itemRenderSize.x();
            itemRenderOffset.x() = renderOffset.x();
            renderSize.x() = itemRenderSize.x();
        }
        else if (HorizontalContentAlignment == AlignmentType::Start)
        {
            itemRenderOffset.x() = renderOffset.x() - renderSize.x() + itemRenderSize.x();
        }
        if (renderSize.y() < itemRenderSize.y())
        {
            renderOffset.y() = renderOffset.y() - renderSize.y() + itemRenderSize.y();
            itemRenderOffset.y() = renderOffset.y();
            renderSize.y() = itemRenderSize.y();
        }
        item->RenderOffset(itemRenderOffset);
        if (itemRenderSize.x() == 0.f || itemRenderSize.y() == 0.f)
            item->RenderSize(renderSize);
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void ItemPresenter::PointerEventTrigger(PointerEventArgs& args)
{
    Control::PointerEventTrigger(args);
    if (item != nullptr)
        item->PointerEventTrigger(args);
}
