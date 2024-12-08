#include "Headers/Slider.hpp"

using namespace AnA;
using namespace Controls;

#define SLIDER_SIZE 0.02f
#define SLIDER_HALF_SIZE SLIDER_SIZE * 0.5f

Slider::Slider()
{
    Color = {0.8, 0.8, 0.8};
    button.Color = {0.6, 0.6, 0.6};
}

void Slider::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    int o = Orientation == Horizontal ? 0 : 1;
    int invO = 1 - o;
    auto size = RenderSize();
    auto offset = RenderOffset();
    ((float*)&offset)[o] += (Value - 0.5f) * 2.0f * ((float*)&size)[o] - SLIDER_HALF_SIZE;
    button.RenderSize({SLIDER_SIZE, SLIDER_SIZE});
    button.RenderOffset(offset);
    button.ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}