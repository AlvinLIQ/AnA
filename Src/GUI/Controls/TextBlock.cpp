#include "Headers/TextBlock.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
    //Properties.sType = ANA_TEXT;
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
    //exture = std::make_unique<AnA::Texture>(newText, 0, 0, 128.0, Control::GetDevice());
}

const char* TextBlock::Text()
{
    return text.c_str();
}