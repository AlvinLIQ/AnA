#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/Texture.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
}

TextBlock::~TextBlock()
{
    if (texture)
        delete texture;
}

void TextBlock::PrepareDraw()
{
    
}

void TextBlock::Text(const char* newText)
{
    TextBlock::~TextBlock();
    text = newText;
    texture = new Texture(newText, 0, 0, 32.0, Control::GetDevice());
}

const char* TextBlock::Text()
{
    return text.c_str();
}