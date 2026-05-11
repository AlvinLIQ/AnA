#include "Headers/StackView.hpp"
#include "Headers/Control.hpp"
#include "Headers/ItemsPresenter.hpp"

using namespace AnA;
using namespace Controls;

StackView::StackView()
{
    RenderMode(Absolute);
}

StackView::~StackView()
{

}

void StackView::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void StackView::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    GetActualControlOffset();
    for (auto& item : items)
    {
        item->Aspect = Aspect;
        item->Extent = Extent;
        item->GetSizeForRender();
        item->GetActualControlOffset();
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}
