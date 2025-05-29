#include "Headers/ItemPresenter.hpp"
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
    Control::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
    if (item != nullptr)
    {
        item->HorizontalAlignment = HorizontalContentAlignment;
        item->VerticalAlignment = VerticalContentAlignment;
        item->PrepareDraw(shapeBuffer, imageInfos, shapeCount);
    }
}

void ItemPresenter::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    if (item != nullptr)
    {
        item->Extent = Extent;
        item->Aspect = Aspect;
        auto itemRenderSize = item->GetSizeForRender();
        item->RenderSize(itemRenderSize);
        auto renderSize = RenderSize();
        auto renderOffset = RenderOffset();
        if (renderSize.x() < itemRenderSize.x())
        {
            renderOffset.x() = renderOffset.x() - renderSize.x() + itemRenderSize.x();
            renderSize.x() = itemRenderSize.x();
        }
        if (renderSize.y() < itemRenderSize.y())
        {
            renderOffset.y() = renderOffset.y() - renderSize.y() + itemRenderSize.y();
            renderSize.y() = itemRenderSize.y();
        }
        RenderSize(renderSize);
        RenderOffset(renderOffset);
        item->RenderOffset(RenderOffset());
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void ItemPresenter::PointerEventTrigger(PointerEventArgs& args)
{
    if (item != nullptr)
        item->PointerEventTrigger(args);
}