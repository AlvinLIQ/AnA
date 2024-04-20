//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/Button.hpp"
#include "Styles/Default/ControlStyle.hpp"

using namespace AnA;
using namespace AnA::Controls;

void Button_PointerEntered(Button* control, PointerEventArgs& args)
{
    control->Color = ButtonPointerMovedBackgroundColor;
}

void Button_PointerExited(Button* control, PointerEventArgs& args)
{
    control->Color = ButtonBackgroundColor;
}

void Button_PointerPressed(Button* control, PointerEventArgs& args)
{
    args.Handled = true;
    control->Color = ButtonPointerPressedBackgroundColor;
}

void Button_PointerReleased(Button* control, PointerEventArgs& args)
{
    args.Handled = true;
    control->Color = control->IsInside(args.Position) ? ButtonPointerMovedBackgroundColor : ButtonBackgroundColor;
}

Button::Button() : ItemPresenter()
{
    SetRenderMode(ButtonRenderMode);
    ControlSize = ButtonMinSize;
    Color = ButtonBackgroundColor;
    PointerEvents[PointerEventType::Entered].push_back((PointerEventHandler)Button_PointerEntered);
    PointerEvents[PointerEventType::Exited].push_back((PointerEventHandler)Button_PointerExited);
    PointerEvents[PointerEventType::Pressed].push_back((PointerEventHandler)Button_PointerPressed);
    PointerEvents[PointerEventType::Released].push_back((PointerEventHandler)Button_PointerReleased);
}
