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

void StackView::PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount)
{
    ApplyRenderInfo(shapeBuffer, shapeCount);
}

void StackView::ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount)
{
    GetActualControlOffset();
    for (auto& item : items)
    {
        item->Aspect = Aspect;
        item->Extent = Extent;
        item->GetSizeForRender();
        item->GetActualControlOffset();
        item->ApplyRenderInfo(shapeBuffer, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, shapeCount);
}
