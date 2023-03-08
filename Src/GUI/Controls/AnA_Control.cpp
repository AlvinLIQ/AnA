#include "Headers/AnA_Control.hpp"
#include "../../Core/Headers/AnA_App.hpp"

using namespace AnA;
using namespace AnA::Controls;

AnA_SwapChain *aSwapChain = nullptr;
AnA_Control *pressedControl = nullptr;
AnA_Control *focusedControl = nullptr;

AnA_Control::AnA_Control() : AnA_Object()
{
    this->Model = AnA_App::Get2DModel();
}

void AnA_Control::PrepareDraw()
{
    auto renderSize = GetSizeForRender();
    Properties.transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    
    auto renderOffset = GetActualControlOffset(renderSize);
    Properties.transform.translation = {renderOffset.x, renderOffset.y, 0.f};
}

void AnA_Control::InitControl(AnA_SwapChain *swapChain)
{
    aSwapChain = swapChain;
}

VkExtent2D AnA_Control::GetSwapChainExtent()
{
    return aSwapChain->GetExtent();
}

bool AnA_Control::IsFocused()
{
    return focusedControl == this;
}

void AnA_Control::Focus()
{
    focusedControl = this;
}

void AnA_Control::Unfocus()
{
    if (IsFocused())
        focusedControl = nullptr;
}