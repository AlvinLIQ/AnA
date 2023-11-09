#include "Headers/TextBlock.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
    Properties.sType = ANA_TEXT;
}

TextBlock::~TextBlock()
{

}

void TextBlock::PrepareDraw()
{
    
}

void TextBlock::Text(const char* newText)
{
    text = newText;
    Texture = std::make_shared<AnA::Texture>(newText, 800, 450, 128.0, Control::GetDevice());
}

const char* TextBlock::Text()
{
    return text.c_str();
}