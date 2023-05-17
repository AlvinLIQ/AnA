#include "Headers/Control.hpp"
#include "../../Core/Headers/App.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain *aSwapChain = nullptr;
Control *pressedControl = nullptr;
Control *focusedControl = nullptr;

Control::Control() : Object()
{
    this->Model = App::Get2DModel();
}

void Control::PrepareDraw()
{
    auto renderSize = GetSizeForRender();
    Properties.transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    
    auto renderOffset = GetActualControlOffset(renderSize);
    Properties.transform.translation = {renderOffset.x, renderOffset.y, 0.f};
}

void Control::InitControl(SwapChain *swapChain)
{
    aSwapChain = swapChain;
}

VkExtent2D Control::GetSwapChainExtent()
{
    return aSwapChain->GetExtent();
}

bool Control::IsFocused()
{
    return focusedControl == this;
}

void Control::Focus()
{
    focusedControl = this;
}

void Control::Unfocus()
{
    if (IsFocused())
        focusedControl = nullptr;
}