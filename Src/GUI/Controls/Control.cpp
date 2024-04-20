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

void Control::PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset(renderSize);
    this->Transform.scale = {size.Width, size.Height, 1.f};
    this->Transform.translation = {offset.x, offset.y, 0.f};
    shapeBuffer[shapeCount].transform = Transform.mat4();
    shapeBuffer[shapeCount].color = Color;
    shapeCount++;
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

void Control::PointerEventTrigger(PointerEventArgs& args)
{
    if (args.Handled)
        return;
    PointerEventType eventType = args.EventType;
    bool isInside = IsInside(args.Position);
    printf("%.2f %.2f %d\r", args.Position.x, args.Position.y, isInside ? 1 : 0);
    if (eventType == PointerEventType::Moving)
    {
        if (cursorInside && !isInside)
            eventType = PointerEventType::Exited;
        else if (!cursorInside && isInside)
            eventType = PointerEventType::Entered;
        else if (!cursorInside && !isInside)
            return;
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
    cursorConfig.callBack = [](void* param, CursorPosition& pos)
    {
        auto control = (Control*)param;
        PointerEventArgs args;
        args.EventType = PointerEventType::Moving;
        args.TriggerType = PointerTriggerType::Mouse;
        auto extent = Control::GetSwapChainExtent();
        args.Position = {pos.x / ((float)control->Extent.width / (float)extent.width), 
                        pos.y / ((float)control->Extent.height / (float)extent.height)};
        control->PointerEventTrigger(args);
        auto shapeBuffer = Resource::ResourceManager::GetCurrent()->Shapes->GetBufferData();
        uint32_t shapeCount = 0;
        control->PrepareDraw(shapeBuffer, shapeCount);
    };
    profile.cursorConfigs.push_back(cursorConfig);
    profiles.push_back(profile);
}