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

void ItemPresenter::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfo, uint32_t& shapeCount)
{
    Control::PrepareDraw(shapeBuffer, imageInfo, shapeCount);
    if (item != nullptr)
        item->PrepareDraw(shapeBuffer, imageInfo, shapeCount);
}