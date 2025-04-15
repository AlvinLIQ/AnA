#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/Texture.hpp"

using namespace AnA;
using namespace AnA::Controls;

TextBlock::TextBlock()
{
    Color = {};
    RenderMode(Absolute);
    ControlSize = {75.0f, ANA_TEXT_DEFAULT_LINE_HEIGHT};
}

TextBlock::TextBlock(const char* text, glm::vec3 color)
{
    Color = color;
    RenderMode(Absolute);
    ControlSize = {75.0f, ANA_TEXT_DEFAULT_LINE_HEIGHT};
    Text(text);
}

TextBlock::~TextBlock()
{
    delete texture;
}

void TextBlock::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset();
    this->Transform.scale = {size.Width, size.Height, 1.f};
    this->Transform.translation = {offset.x, offset.y, 0.f};
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void TextBlock::Text(const char* newText)
{
    if (texture)
        delete texture;
    text = newText;
    int width = 0, height = 0;
    //float* scale = GetScale();
    texture = new Texture(text.Str(), width, height, 0, Control::GetDevice());
    ControlSize = {(float)width, (float)height};
}

const char* TextBlock::Text()
{
    return text.Str();
}

VkDescriptorImageInfo TextBlock::GetDescriptorImageInfo()
{
    if (texture)
        return texture->GetImageInfo();
    else
        return Control::GetDescriptorImageInfo();
}