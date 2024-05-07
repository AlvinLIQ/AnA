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

void ItemPresenter::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    Control::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
    if (item != nullptr)
    {
        item->HorizontalAlignment = AlignmentType::Center;
        item->VerticalAlignment = AlignmentType::Center;
        item->PrepareDraw(shapeBuffer, imageInfos, shapeCount);
    }
}

void ItemPresenter::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    if (item != nullptr)
    {
        item->RenderMode(RenderMode());
        item->RenderOffset(RenderOffset());
        item->RenderSize(RenderSize());
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}