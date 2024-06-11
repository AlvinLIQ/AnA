#include "Headers/TextBox.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBox::TextBox()
{
    
}

void TextBox::Insert(size_t index, uint32_t ch)
{
    text.insert(text.begin() + index, ch);
}

void TextBox::CharacterRecevied(uint32_t ch)
{
    Insert(cursor, ch);
}