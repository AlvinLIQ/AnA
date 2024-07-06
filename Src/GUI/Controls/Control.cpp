#include "Headers/Control.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;

Control::Control()
{
}

Control::~Control()
{
    
}

POS_F Control::GetActualControlOffset(SIZE_F renderSize)
{
    float* pOffset = (float*)&renderOffset;
    float* pSize = (float*)&this->renderSize;
    AlignmentType Alignments[]{HorizontalAlignment, VerticalAlignment};
    for (int i = 0; i < 2; i++)
    {
        if (Alignments[i] == AlignmentType::Start)
            pOffset[i] = pSize[i] / 2.f - 1.0f;
        else if (Alignments[i] == AlignmentType::End)
            pOffset[i] = 1.0f - pSize[i] / 2.f;
        else
        {
            pOffset[i] = 0.f;
            if (Alignments[i] == AlignmentType::Stretch)
                pSize[i] = 1.f;
        }
    }
                
    renderOffset.x += ControlOffset.x;
    renderOffset.y += ControlOffset.y;
    renderOffset.x *= aSwapChain->ScaleX;
    renderOffset.y *= aSwapChain->ScaleY;
    return renderOffset;
}

SIZE_F Control::GetSizeForRender()
{
    if (renderMode == AlignType::Absolute)
    {
        renderSize.Width = ControlSize.Width / (float)Extent.width;
        renderSize.Height = ControlSize.Height / (float)Extent.height;
    }
    else if (renderMode == AlignType::Relative)
    {
        renderSize = ControlSize;
    }
    else if (renderMode == AlignType::Auto)
    {
        renderSize = {ControlSize.Width / Aspect, ControlSize.Height};
    }

    renderSize.Width *= aSwapChain->ScaleX;
    renderSize.Height *= aSwapChain->ScaleY;
    return renderSize;
}

void Control::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset(renderSize);
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void Control::InitControl(SwapChain* swapChain)
{
    aSwapChain = swapChain;
}

VkExtent2D Control::GetSwapChainExtent()
{
    return aSwapChain->GetExtent();
}

Device* Control::GetDevice()
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

bool Control::IsInside(CursorPosition pos)
{
    POS_F offset = {renderOffset.x * 0.5f + 0.5f - renderSize.Width * 0.5f, renderOffset.y * 0.5f + 0.5f - renderSize.Height * 0.5f};
    if (HorizontalAlignment == Stretch)
    {
        offset.x = 0.0f;
    }
    if (VerticalAlignment == Stretch)
    {
        offset.y = 0.0f;
    }
    return IsInside(pos, offset, renderSize);
}

bool Control::IsInside(CursorPosition& pos, POS_F& offset, SIZE_F& size)
{
    return pos.x >= offset.x && pos.y >= offset.y && pos.x <= offset.x + size.Width && pos.y <= offset.y + size.Height;
}

VkDescriptorImageInfo Control::GetDescriptorImageInfo()
{
    return Resource::ResourceManager::GetCurrent()->TextureMap.at(TextureId).GetImageInfo();
}

void Control::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    this->Transform.scale = {renderSize.Width, renderSize.Height, 1.f};
    this->Transform.translation = {renderOffset.x, renderOffset.y, 0.f};
    shapeBuffer[shapeCount].transform = Transform.mat4();
    shapeBuffer[shapeCount].color = Color;
    if (imageInfos.size() <= shapeCount)
    {
        imageInfos.resize(shapeCount + 1);
    }
    imageInfos[shapeCount] = this->GetDescriptorImageInfo();
    shapeCount++;
}

void Control::PointerEventTrigger(PointerEventArgs& args)
{
    if (args.Handled)
        return;
    PointerEventType eventType = args.EventType;
    bool isInside = IsInside(args.Position);
    if (eventType == PointerEventType::Moving)
    {
        if (cursorInside && !isInside)
            eventType = PointerEventType::Exited;
        else if (cursorInside && pressed)
        {
            eventType = PointerEventType::Released;
            pressed = false;
        }
        else if (!cursorInside && isInside)
            eventType = PointerEventType::Entered;
        else if (!cursorInside && !isInside)
            return;
    }
    else if (eventType == PointerEventType::Pressed)
    {
        if (!isInside)
        {
            if (cursorInside)
                eventType = PointerEventType::Exited;
            else
                return;
        }
        else
        {
            pressed = true;
        }
    }
    cursorInside = isInside;
    for (auto event : PointerEvents[eventType])
    {
        event(this, args);  
    }
}

void Controls::Control::GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles)
{
    Input::InputProfile profile{};
    profile.flag = Input::InputProfileFlags::None;
    Input::CursorConfig cursorConfig{};
    cursorConfig.param = mainControl;
    cursorConfig.callBack = [](void* param, CursorPosition& pos, int leftButtonAction)
    {
        auto control = (Control*)param;
        PointerEventArgs args;
        args.EventType = leftButtonAction == GLFW_PRESS ? PointerEventType::Pressed : PointerEventType::Moving;
        args.TriggerType = PointerTriggerType::Mouse;
        auto extent = Control::GetSwapChainExtent();
        args.Position = {pos.x / ((float)control->Extent.width / (float)extent.width), 
                        pos.y / ((float)control->Extent.height / (float)extent.height)};
        control->PointerEventTrigger(args);
        Resource::ResourceManager::GetCurrent()->Shapes.PrepareDraw(control);
    };
    profile.cursorConfigs.push_back(cursorConfig);
    profiles.push_back(profile);
}