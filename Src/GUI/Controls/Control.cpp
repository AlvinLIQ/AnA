#include "Headers/Control.hpp"
#include "../../Core/Headers/App.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;

Control::Control()
{
    this->Model = App::Get2DModel();
}

Control::~Control()
{
    
}

void Control::Draw(VkCommandBuffer commandBuffer)
{
    auto renderSize = GetSizeForRender();
    //Properties.transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    //ControlSize = renderSize;
    auto renderOffset = GetActualControlOffset(renderSize);
    //ControlOffset = renderOffset;
    this->Transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    this->Transform.translation = {renderOffset.x, renderOffset.y, 0.f};
    ObjectPushConstantData push{this->Transform.mat4(), Color};
    auto& shader = Resource::ResourceManager::GetCurrent()->Shaders[1];
    auto& pipelineLayout = shader.GetPipelineLayout();
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push), &push);
    Object::Draw(commandBuffer);
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