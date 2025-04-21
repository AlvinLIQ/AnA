//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/Button.hpp"
#include "Styles/Default/ControlStyle.hpp"

using namespace AnA;
using namespace AnA::Controls;

void Button_PointerEntered(Button* control, PointerEventArgs& )
{
    control->Color = ButtonPointerMovedBackgroundColor;
}

void Button_PointerExited(Button* control, PointerEventArgs& )
{
    control->Color = ButtonBackgroundColor;
}

void Button_PointerPressed(Button* control, PointerEventArgs& args)
{
    if (!control->IsInside(args.Position))
        return;
    control->Focus();
    args.Handled = true;
    control->Color = ButtonPointerPressedBackgroundColor;
}

void Button_PointerReleased(Button* control, PointerEventArgs& args)
{
    control->Color = control->IsInside(args.Position) ? ButtonPointerMovedBackgroundColor : ButtonBackgroundColor;
    args.Handled = false;
}

Button::Button() : ItemPresenter()
{
    RenderMode(Absolute);
    ControlSize = ButtonMinAbsoluteSize;
    Color = ButtonBackgroundColor;
    PointerEvents[PointerEventType::Entered].push_back(reinterpret_cast<PointerEventHandler>(Button_PointerEntered));
    PointerEvents[PointerEventType::Exited].push_back(reinterpret_cast<PointerEventHandler>(Button_PointerExited));
    PointerEvents[PointerEventType::Pressed].push_back(reinterpret_cast<PointerEventHandler>(Button_PointerPressed));
    PointerEvents[PointerEventType::Released].push_back(reinterpret_cast<PointerEventHandler>(Button_PointerReleased));
}
