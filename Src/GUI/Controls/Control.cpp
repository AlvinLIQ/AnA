#include "Headers/Control.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"
#include "../../Core/Headers/App.hpp"

using namespace AnA;
using namespace AnA::Controls;

SwapChain* aSwapChain = nullptr;
Control* pressedControl = nullptr;
Control* focusedControl = nullptr;
bool leftButtonPressed = false;
short needUpdate = 0;
bool textLayoutNeedReset = false;
Vec2 lastPressedPos{};
bool dragStarted = false;

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

void Control::PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount)
{
    GetSizeForRender();
    GetActualControlOffset();
    ApplyRenderInfo(shapeBuffer, shapeCount);
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
    if (focusedControl != this)
    {
        if (focusedControl)
            focusedControl->Unfocus();
        focusedControl = this;
    }
}

void Control::Unfocus()
{
    if (IsFocused())
        focusedControl = nullptr;
}

void Control::ClearFocus()
{
    if (focusedControl)
        focusedControl->Unfocus();
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
    needUpdate = 3;
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

void Control::ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount)
{
    Vec2 maxBounding = renderOffset + renderSize;
    if (Parent && (Parent->renderOffset.x() + Parent->renderSize.x() < renderOffset.x() ||
        Parent->renderOffset.y() + Parent->renderSize.y() < renderOffset.y() ||
        Parent->renderOffset.x() > maxBounding.x() || Parent->renderOffset.y() > maxBounding.y()))
    {
        visible = false;
        return;
    }

    if (renderOffset.x() < -renderSize.x() || renderOffset.y() < -renderSize.y() ||
        renderOffset.x() > 1.0f || renderOffset.y() > 1.0f)
    {
        visible = false;
        return;
    }

    this->scale = {renderSize.x(), renderSize.y()};
    this->translation = {renderOffset.x() * 2.0f - 1.0f + renderSize.x(), renderOffset.y() * 2.0f - 1.0f + renderSize.y()};
    this->bounding.x = renderOffset.x();
    this->bounding.y = renderOffset.y();
    this->bounding.z = maxBounding.x();
    this->bounding.w = maxBounding.y();
    if (Parent)
    {
        bounding.x = std::max(bounding.x, Parent->bounding.x);//Parent->renderOffset.x().value);//Parent->renderOffset.x().value * float(Parent->Extent.width));
        bounding.y = std::max(bounding.y, Parent->bounding.y);//Parent->renderOffset.y().value);//Parent->renderOffset.y().value * float(Parent->Extent.height));
        bounding.z = std::min(bounding.z, Parent->bounding.z);//Parent->renderOffset.x().value + Parent->renderSize.x().value);//maxBounding.x().value * float(Parent->Extent.width));
        bounding.w = std::min(bounding.w, Parent->bounding.w);//Parent->renderOffset.y().value + Parent->renderSize.y().value);//maxBounding.y().value * float(Parent->Extent.height));
    }

    shapeBuffer[shapeCount].scale = this->scale;
    shapeBuffer[shapeCount].translation = this->translation;
    shapeBuffer[shapeCount].bounding = this->bounding;
    shapeBuffer[shapeCount].color = Color;
    shapeBuffer[shapeCount].texIndex = Resources::ResourceManager::GetCurrent()->TextureIdMap[TextureId];
    shapeBuffer[shapeCount].texLayer = TextureLayer;

    shapeId = shapeCount;
    shapeCount++;
    visible = true;
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
    case DragBegan:
        result = isInside;
        pressed = true;
        break;
    case DragEnded:
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

void Control::TextReceived(const char* text)
{
    printf("%s\n", text);
}

PointerEventType GetPointerEventType(int buttonAction, const Input::CursorArgs& args)
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
        else if (std::abs(args.duration.x.value) + std::abs(args.duration.y.value) > std::numeric_limits<double>::epsilon())
        {
            if (!dragStarted)
            {
                dragStarted = true;
                eventType = PointerEventType::DragBegan;
            }
            else
            {
                eventType = PointerEventType::Dragging;
            }

        }
        break;
    case ANA_RELEASE:
        if (leftButtonPressed)
        {
            leftButtonPressed = false;
            if (dragStarted)
            {
                dragStarted = false;
                eventType = PointerEventType::DragEnded;
            }
            else
                eventType = PointerEventType::Released;
            if (focusedControl != nullptr && !focusedControl->FocusType)
                Control::ClearFocus();
        }

        break;
    default:
        break;
    }
    return eventType;
}

void _textRecevied(const char* text)
{
    if (focusedControl != nullptr)
        focusedControl->TextReceived(text);
}

void _scrolled(float dx, float dy)
{
    auto mainControl = Resources::ResourceManager::GetCurrent()->MainControl;
    PointerEventArgs args;
    args.EventType = Scrolled;
    args.Position = Controls::Control::GetRelativePosition(App::GetCurrent()->GetWindow().GetCursorPos(), mainControl->Extent);
    args.Duration = Controls::Control::GetRelativePosition({(double)dx * 0.04f, (double)dy * 0.04f}, mainControl->Extent);
    if (focusedControl)
        focusedControl->PointerEventTrigger(args);
    else
        mainControl->PointerEventTrigger(args);
    /*
    mainControl->ControlOffset.x() += args.Duration.x.As<float>();
    mainControl->ControlOffset.y() += args.Duration.y.As<float>();
    mainControl->RequestUpdate();*/
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

        args.EventType = GetPointerEventType(leftButtonAction, curArgs);
        args.TriggerType = PointerTriggerType::Mouse;
        args.Position = GetRelativePosition(curArgs.pos, control->Extent);

        args.Duration = curArgs.duration;
        if (focusedControl)
            focusedControl->PointerEventTrigger(args);
        else
            control->PointerEventTrigger(args);
    };
    profile.cursorConfigs.push_back(cursorConfig);
    profile.textConfigs.push_back({_textRecevied});
    profile.scrollConfigs.push_back({_scrolled});
    profiles.push_back(profile);
}

CursorPosition Controls::Control::GetRelativePosition(const CursorPosition& pos, const VkExtent2D& extent)
{
    auto _extent = Control::GetSwapChainExtent();
    #ifdef _WIN32
        return {pos.x / (extent.width / static_cast<double>(_extent.width)),
                    pos.y / (extent.height / static_cast<double>(_extent.height))};
    #else
        auto scale = Control::GetScale();
        return {pos.x * scale[0].As<double>() / (extent.width / static_cast<double>(_extent.width)),
                    pos.y * scale[1].As<double>() / (extent.height / static_cast<double>(_extent.height))};
    #endif
}
