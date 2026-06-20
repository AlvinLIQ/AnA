#include "Headers/Control.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;
bool leftButtonPressed = false;
short needUpdate = 0;
bool textLayoutNeedReset = false;
Vec2 lastPressedPos{};

Control::Control()
{
}

Control::~Control()
{

}

Vec2 Control::GetActualControlOffset()
{
    float* pOffset = reinterpret_cast<float*>(&renderOffset);
    float* pSize = reinterpret_cast<float*>(&this->renderSize);
    AlignmentType Alignments[]{HorizontalAlignment, VerticalAlignment};
    Vec2 maxSize = Parent ? Parent->ContentRenderSize() : Vec2(1.0f, 1.0f);
    for (int i = 0; i < 2; i++)
    {
        if (Alignments[i] == AlignmentType::Center)
            pOffset[i] = 0.5f - pSize[i] * 0.5f;
        else if (Alignments[i] == AlignmentType::End)
            pOffset[i] = 1.0f - pSize[i];
        else
        {
            pOffset[i] = 0.0f;
            if (Alignments[i] == AlignmentType::Stretch)
                pSize[i] = maxSize[i];
        }

    }

    renderOffset.x() += ControlOffset.x();
    renderOffset.y() += ControlOffset.y();
    //renderOffset.x() *= aSwapChain->ScaleX;
    //renderOffset.y() *= aSwapChain->ScaleY;
    return renderOffset;
}

Vec2 Control::GetSizeForRender()
{
    if (renderMode == AlignType::Absolute)
    {
        renderSize.x() = ControlSize.x() / static_cast<float>(Extent.width);
        renderSize.y() = ControlSize.y() / static_cast<float>(Extent.height);
        renderSize.x() *= aSwapChain->ScaleX;
        renderSize.y() *= aSwapChain->ScaleY;
    }
    else if (renderMode == AlignType::Relative)
    {
        renderSize = ControlSize;
    }
    else if (renderMode == AlignType::Auto)
    {
        renderSize = {ControlSize.x() / Aspect, ControlSize.y()};
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

Float* Control::GetScale()
{
    return aSwapChain->Scale;
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
    Vec2 offset = ActualRenderOffset();
    return IsInside(pos, offset, renderSize);
}

bool Control::IsInside(CursorPosition& pos, Vec2& offset, Vec2& size)
{
    return pos.x.As<float>() >= offset.x() && pos.y.As<float>() >= offset.y() &&
    pos.x.As<float>() <= offset.x() + size.x() && pos.y.As<float>() <= offset.y() + size.y();
}

bool Control::NeedUpdate()
{
    return needUpdate;
}

void Control::RequestUpdate()
{
    needUpdate = 2;
}

void Control::EndUpdate()
{
    needUpdate--;
}

bool Control::TextLayoutNeedReset()
{
    return textLayoutNeedReset;
}

void Control::RequestTextLayoutReset()
{
    textLayoutNeedReset = true;
}

void Control::EndTextLayoutReset()
{
    textLayoutNeedReset = false;
}

void Control::Texture(const std::string path)
{
    TextureId = Resources::ResourceManager::GetCurrent()->AppendTexture(path);
}

VkDescriptorImageInfo Control::GetDescriptorImageInfo()
{
    return Resources::ResourceManager::GetCurrent()->TextureMap.at(TextureId).GetImageInfo();
}

void Control::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    if (renderOffset.x() < -renderSize.x() || renderOffset.y() < -renderSize.y() ||
        renderOffset.x() + renderSize.x() > 1.0f || renderOffset.y() + renderSize.y() > 1.0f)
        return;

    this->scale = {renderSize.x(), renderSize.y()};
    this->translation = {renderOffset.x() * 2.0f - 1.0f + renderSize.x(), renderOffset.y() * 2.0f - 1.0f + renderSize.y()};
    this->bounding.x = renderOffset.x() * float(Extent.width);
    this->bounding.y = renderOffset.y() * float(Extent.height);
    this->bounding.z = this->bounding.x + renderSize.x() * float(Extent.width);
    this->bounding.w = this->bounding.y + renderSize.y() * float(Extent.height);

    shapeBuffer[shapeCount].scale = this->scale;
    shapeBuffer[shapeCount].translation = this->translation;
    shapeBuffer[shapeCount].bounding = this->bounding;
    shapeBuffer[shapeCount].color = Color;
    shapeBuffer[shapeCount].texLayer = TextureLayer;
    if (imageInfos.size() <= shapeCount)
    {
        imageInfos.resize(shapeCount + MaxBatchSize);
    }
    imageInfos[shapeCount] = this->GetDescriptorImageInfo();
    shapeId = shapeCount;
    shapeCount++;
}

inline void RunPointerEvents(std::vector<PointerEventHandler>& events, void* param, PointerEventArgs& args)
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
            result = IsFocused();
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

void Control::CharacterRecevied(uint32_t ch)
{
    printf("%c", ch);
}

PointerEventType GetPointerEventType(int buttonAction)
{
    PointerEventType eventType = PointerEventType::Moving;
    switch (buttonAction)
    {
    case ANA_PRESS:
        if (!leftButtonPressed)
        {
            leftButtonPressed = true;
            eventType = PointerEventType::Pressed;
            if (focusedControl != nullptr && focusedControl->FocusType)
                Control::ClearFocus();
        }
        break;
    case ANA_RELEASE:
        if (leftButtonPressed)
        {
            leftButtonPressed = false;
            eventType = PointerEventType::Released;
            if (focusedControl != nullptr && !focusedControl->FocusType)
                Control::ClearFocus();
        }
    default:
        break;
    }
    return eventType;
}

void _characterReceived(uint32_t ch)
{
    if (focusedControl != nullptr)
        focusedControl->CharacterRecevied(ch);
}

void Controls::Control::GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles)
{
    Input::InputProfile profile{};
    profile.flag = Input::InputProfileFlags::None;
    Input::CursorConfig cursorConfig{};
    cursorConfig.param = mainControl;
    cursorConfig.callback = [](void* param, Input::CursorArgs& curArgs, int leftButtonAction)
    {
        auto control = static_cast<Control*>(param);
        PointerEventArgs args;

        args.EventType = GetPointerEventType(leftButtonAction);
        args.TriggerType = PointerTriggerType::Mouse;
        auto extent = Control::GetSwapChainExtent();
#ifdef _WIN32
        args.Position = {curArgs.pos.x / (control->Extent.width / static_cast<double>(extent.width)),
                        curArgs.pos.y / (control->Extent.height / static_cast<double>(extent.height))};
#else
        auto scale = Control::GetScale();
        args.Position = {curArgs.pos.x * scale[0].As<double>() / (control->Extent.width / static_cast<double>(extent.width)),
                        curArgs.pos.y * scale[1].As<double>() / (control->Extent.height / static_cast<double>(extent.height))};
#endif
        args.Duration = curArgs.duration;
        if (focusedControl)
            focusedControl->PointerEventTrigger(args);
        else
            control->PointerEventTrigger(args);
    };
    profile.cursorConfigs.push_back(cursorConfig);
    profile.characterConfigs.push_back({_characterReceived});
    profiles.push_back(profile);
}
