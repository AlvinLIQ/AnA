#include "Headers/Control.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;
bool leftButtonPressed = false;
POS_2F lastPressedPos{};

Control::Control()
{
}

Control::~Control()
{
    
}

POS_2F Control::GetActualControlOffset()
{
    float* pOffset = reinterpret_cast<float*>(&renderOffset);
    float* pSize = reinterpret_cast<float*>(&this->renderSize);
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

SIZE_2F Control::GetSizeForRender()
{
    if (renderMode == AlignType::Absolute)
    {
        renderSize.Width = ControlSize.Width / static_cast<float>(Extent.width);
        renderSize.Height = ControlSize.Height / static_cast<float>(Extent.height);
        renderSize.Width *= aSwapChain->ScaleX;
        renderSize.Height *= aSwapChain->ScaleY;
    }
    else if (renderMode == AlignType::Relative)
    {
        renderSize = ControlSize;
    }
    else if (renderMode == AlignType::Auto)
    {
        renderSize = {ControlSize.Width / Aspect, ControlSize.Height};
    }

    return renderSize;
}

void Control::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    GetSizeForRender();
    GetActualControlOffset();
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void Control::InitControl(SwapChain* swapChain)
{
    aSwapChain = swapChain;
}

float* Control::GetScale()
{
    return &aSwapChain->Scale[0];
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

void Control::ClearFocus()
{
    focusedControl = nullptr;
}

Control* Control::GetFocused()
{
    return focusedControl;
}

bool Control::IsInside(CursorPosition pos)
{
    POS_2F offset = {renderOffset.x * 0.5f + 0.5f - renderSize.Width * 0.5f, renderOffset.y * 0.5f + 0.5f - renderSize.Height * 0.5f};
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

bool Control::IsInside(CursorPosition& pos, POS_2F& offset, SIZE_2F& size)
{
    return static_cast<float>(pos.x) >= offset.x && static_cast<float>(pos.y) >= offset.y && 
    static_cast<float>(pos.x) <= offset.x + size.Width && static_cast<float>(pos.y) <= offset.y + size.Height;
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
        imageInfos.resize(shapeCount + MaxBatchSize);
    }
    imageInfos[shapeCount] = this->GetDescriptorImageInfo();
    shapeCount++;
}

void RunPointerEvents(std::vector<PointerEventHandler>& events, void* param, PointerEventArgs& args)
{
    for (auto& event : events)
    {
        if (args.Handled)
            break;
        if (event)
            event(param, args);
    }
}

bool Control::ProcessEventArgs(PointerEventArgs& args, PointerEventType& actualEventType)
{
    bool result = true;
    bool isInside = IsInside(args.Position);
    actualEventType = args.EventType;
    switch(args.EventType)
    {
    case Moving:
        if (!cursorInside && isInside)
        {
            actualEventType = Entered;
        }
        else if (cursorInside && !isInside)
        {
            actualEventType = Exited;
        }
        else if (!isInside)
        {
            result = false;
        }
        break;
    case Pressed:
        result = isInside;
        pressed = isInside;
        break;
    case Released:
        result = isInside;
        pressed = false;
        break;
    default:
        result = true;
        break;
    }

    cursorInside = isInside;
    return result;
}

void Control::PointerEventTrigger(PointerEventArgs& args)
{
    PointerEventType actualEventType;
    if (ProcessEventArgs(args, actualEventType))
        RunPointerEvents(PointerEvents[actualEventType], this, args);
}

PointerEventType GetPointerEventType(int buttonAction)
{
    PointerEventType eventType = PointerEventType::Moving;
    switch (buttonAction)
    {
    case GLFW_PRESS:
        if (!leftButtonPressed)
        {
            leftButtonPressed = true;
            eventType = PointerEventType::Pressed;
        }
        break;
    case GLFW_RELEASE:
        if (leftButtonPressed)
        {
            leftButtonPressed = false;
            eventType = PointerEventType::Released;
        }
    default:
        break;
    }
    return eventType;
}

void Controls::Control::GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles)
{
    Input::InputProfile profile{};
    profile.flag = Input::InputProfileFlags::None;
    Input::CursorConfig cursorConfig{};
    cursorConfig.param = mainControl;
    cursorConfig.callBack = [](void* param, CursorPosition& pos, int leftButtonAction)
    {
        auto control = static_cast<Control*>(param);
        PointerEventArgs args;
        
        args.EventType = GetPointerEventType(leftButtonAction);
        args.TriggerType = PointerTriggerType::Mouse;
        auto extent = Control::GetSwapChainExtent();
        auto scale = Control::GetScale();
        args.Position = {pos.x * static_cast<double>(scale[0]) / (control->Extent.width / static_cast<double>(extent.width)), 
                        pos.y * static_cast<double>(scale[1]) / (control->Extent.height / static_cast<double>(extent.height))};
        control->PointerEventTrigger(args);
        Resource::ResourceManager::GetCurrent()->Shapes.PrepareDraw(control);
    };
    profile.cursorConfigs.push_back(cursorConfig);
    profiles.push_back(profile);
}