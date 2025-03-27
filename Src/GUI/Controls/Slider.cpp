#include "Headers/Slider.hpp"

using namespace AnA;
using namespace Controls;

#define SLIDER_SIZE 0.02f
#define SLIDER_HALF_SIZE SLIDER_SIZE * 0.5f

void slider_Click(void* control, PointerEventArgs& args)
{
    auto slider = (Slider*)control;
    int o = slider->Orientation == AnA::Controls::Horizontal ? 0 : 1;
    auto offset = slider->RenderOffset();
    auto size = slider->RenderSize();
    float pos = ((float)((double*)&args.Position)[o]) * (((float*)&size)[o] + ((float*)&offset)[o]) - ((float*)&offset)[o];
    if (pos < 0.0f)
    {
        pos = 0.0f;
    }
    slider->Value = pos / ((float*)&size)[o];
}

Slider::Slider()
{
    Color = {0.7, 0.7, 0.7};
    button.Color = {0.6, 0.6, 0.6};
    PointerEvents[PointerEventType::Pressed].emplace_back(slider_Click);
}

void Slider::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    int o = Orientation == Horizontal ? 0 : 1;
    //int invO = 1 - o;
    auto size = RenderSize();
    auto offset = RenderOffset();
    float pos = std::max(std::min(Value * ((float*)&size)[o] * (((float*)&size)[o] - ((float*)&offset)[o]), ((float*)&size)[o] - SLIDER_HALF_SIZE), SLIDER_HALF_SIZE);
    ((float*)&offset)[o] += (pos - 0.5f) * 2.0f;

    button.GetSizeForRender();
    ((float*)&size)[o] = SLIDER_SIZE;
    button.RenderSize(size);
    button.RenderOffset(offset);
    button.ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);

    //((float*)&size)[invO] = SLIDER_SIZE;
    //RenderSize(size);
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}