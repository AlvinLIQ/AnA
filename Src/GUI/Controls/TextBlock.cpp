#include "Headers/TextBlock.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{

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
    Texture = std::make_shared<AnA::Texture>(newText, 800, 450, 64, Control::GetDevice());
}

const char* TextBlock::Text()
{
    return text.c_str();
}