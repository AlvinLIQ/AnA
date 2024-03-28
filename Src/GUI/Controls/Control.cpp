#include "Headers/Control.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;

Control::Control()
{
    //this->Model = App::Get2DModel();
}

void Control::PrepareDraw()
{
    auto renderSize = GetSizeForRender();
    //Properties.transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    ControlSize = renderSize;
    auto renderOffset = GetActualControlOffset(renderSize);
    ControlOffset = renderOffset;
}

void Control::Draw(VkCommandBuffer commandBuffer)
{
    //Model->Draw(commandBuffer);
}

void Control::InitControl(SwapChain* swapChain)
{
    aSwapChain = swapChain;
}

VkExtent2D Control::GetSwapChainExtent()
{
    return aSwapChain->GetExtent();
}

Device& Control::GetDevice()
{
    return aSwapChain->GetDevice();
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

bool Control::IsInside(POS_F pos)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset(size);
    return pos.x >= offset.x && pos.y >= offset.y && pos.x <= offset.x + size.Width && pos.y <= offset.y + size.Height;
}