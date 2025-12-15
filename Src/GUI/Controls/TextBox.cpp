#include "Headers/TextBox.hpp"

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

void TextBox::CharacterRecevied(uint32_t ch)
{
    Insert(cursor, ch);
}