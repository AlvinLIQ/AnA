#include "Headers/AnA_Control.h"
#include "../../Core/Headers/AnA_App.h"

using namespace AnA;
using namespace AnA::Controls;

AnA_SwapChain *aSwapChain = nullptr;

AnA_Control::AnA_Control() : AnA_Object()
{
    this->Model = AnA_App::Get2DModel();
}

AnA_Control::~AnA_Control()
{
    
}

void AnA_Control::PrepareDraw()
{
    auto renderSize = GetSizeForRender();
    ItemsProperties[0].transform.scale = {renderSize.Width, renderSize.Height, 0.f};

    auto renderOffset = GetActualControlOffset(renderSize);
    ItemsProperties[0].transform.translation = {renderOffset.x, renderOffset.y, 0.f};
}

void AnA_Control::InitControl(AnA_SwapChain *swapChain)
{
    aSwapChain = swapChain;
}

VkExtent2D AnA_Control::GetSwapChainExtent()
{
    return aSwapChain->GetExtent();
}