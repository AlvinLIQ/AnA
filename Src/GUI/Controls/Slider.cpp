#include "Headers/Slider.hpp"

using namespace AnA;
using namespace Controls;

#define SLIDER_SIZE 0.02f
#define SLIDER_HALF_SIZE SLIDER_SIZE * 0.5f

void slider_Click(void* control, PointerEventArgs& args)
{
    auto slider = static_cast<Slider*>(control);
    if (!slider->IsPressed())
        return;
    args.Handled = true;
    slider->Focus();
    int o = slider->Orientation == AnA::Controls::Horizontal ? 0 : 1;
    auto offset = slider->RenderOffset() + slider->ActualRenderOffset() * 0.5;
    auto size = slider->RenderSize();
    const float curPosF = static_cast<float>(reinterpret_cast<double*>(&args.Position)[o]);
    const float sizeF = reinterpret_cast<float*>(&size)[o];
    const float offsetF = reinterpret_cast<float*>(&offset)[o];

    float pos = (curPosF - offsetF / sizeF) / sizeF;
    //printf("pos: %f curPosF: %f offsetF: %f sizeF: %f\r", pos, curPosF, offsetF, sizeF);

    if (pos < 0.0f)
    {
        pos = 0.0f;
    }
    slider->Value(std::min(1.0f, pos));
}

Slider::Slider()
{
    Color = {0.18, 0.18, 0.18, 1.0};
    button.Color = {0.28, 0.28, 0.28, 1.0};
    PointerEvents[PointerEventType::Moving].emplace_back(slider_Click);
}

void Slider::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    int o = Orientation == Horizontal ? 0 : 1;
    //int invO = 1 - o;
    auto size = RenderSize();
    auto offset = RenderOffset();
    auto actualOffset = ActualRenderOffset();
    float* size2F = reinterpret_cast<float*>(&size);
    float* offset2F = reinterpret_cast<float*>(&offset);
    float pos = std::max(std::min(value / size2F[o] * (size2F[o] - offset2F[o] - actualOffset[o]), size2F[o] - SLIDER_HALF_SIZE), SLIDER_HALF_SIZE);
    offset2F[o] += (pos - 0.5f) * 2.0f;

    button.GetSizeForRender();
    size2F[o] = SLIDER_SIZE;
    button.RenderSize(size);
    offset[o] += actualOffset[o] * 2.0f;
    button.RenderOffset(offset);
    button.ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);

    //((float*)&size)[invO] = SLIDER_SIZE;
    //RenderSize(size);
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}
