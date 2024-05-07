#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/Texture.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
    Color = {};
    SetRenderMode(Relative);
    ControlSize = {75.0f, ANA_TEXT_DEFAULT_LINE_HEIGHT};
}

TextBlock::~TextBlock()
{
    delete texture;
}

void TextBlock::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfo, uint32_t& shapeCount)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset(size);
    this->Transform.scale = {size.Width, size.Height, 1.f};
    this->Transform.translation = {offset.x, offset.y, 0.f};
    shapeBuffer[shapeCount].transform = Transform.mat4();
    shapeBuffer[shapeCount].color = Color;
    if (imageInfo.size() <= shapeCount)
    {
        imageInfo.resize(shapeCount + 1);
    }
    imageInfo[shapeCount] = texture->GetImageInfo();
    shapeCount++;
}

void TextBlock::Text(const char* newText)
{
    //TextBlock::~TextBlock();
    text = newText;
    int width = 0, height = 0;
    texture = new Texture(newText, width, height, 0, Control::GetDevice());
    ControlSize = {(float)width, (float)height};
}

const char* TextBlock::Text()
{
    return text.c_str();
}

VkDescriptorImageInfo TextBlock::GetDescriptorImageInfo()
{
    if (texture)
        return texture->GetImageInfo();
    else
        return Control::GetDescriptorImageInfo();
}