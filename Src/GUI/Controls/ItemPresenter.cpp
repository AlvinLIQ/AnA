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

void ItemPresenter::PrepareDraw()
{
    if (item != nullptr)
        item->PrepareDraw();
}

void ItemPresenter::Draw(VkCommandBuffer commandBuffer)
{
    if (item != nullptr)
        item->Draw(commandBuffer);
}