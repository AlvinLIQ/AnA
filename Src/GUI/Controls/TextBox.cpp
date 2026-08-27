#include "Headers/TextBox.hpp"
#include "../../Core/Headers/App.hpp"

using namespace AnA;
using namespace AnA::Controls;


void TextBox_PointerPressed(TextBox* control, PointerEventArgs& )
{
    control->Focus();
    Control::RequestUpdate();
}

TextBox::TextBox()
{
    FocusType = 1;
    PointerEvents[PointerEventType::Pressed].push_back(reinterpret_cast<PointerEventHandler>(TextBox_PointerPressed));
}

void TextBox::TextReceived(const char* text)
{
    Insert(cursor, text);
}

void TextBox::Focus()
{
    Color = DefaultPrimaryColor;
    auto& window = App::GetCurrent()->GetWindow();
    auto curPos = window.GetCursorPos();
    curPos.x /= Double(window.Width);
    curPos.y /= Double(window.Height);
    if (IsInside(curPos))
    {
        //uint32_t row = uint32_t((curPos.y.As<float>() - renderOffset.y()));
    }
    SDL_StartTextInput(window.GetSDLWindow());
    Control::Focus();
}

void TextBox::Unfocus()
{
    Color = DefaultControlBackgroundColor;
    auto& window = App::GetCurrent()->GetWindow();
    SDL_StopTextInput(window.GetSDLWindow());
    Control::Unfocus();
}
