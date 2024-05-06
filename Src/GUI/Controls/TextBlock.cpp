#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/Texture.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
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
    texture = new Texture(newText, 0, 0, 0, Control::GetDevice());
}

const char* TextBlock::Text()
{
    return text.c_str();
}