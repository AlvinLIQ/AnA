#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace AnA::Controls;

const Vec2 charSize = {0.836363613f, 0.99999994f};
const Vec2 widthCharSize = {0.836363613f, 0.99999994f};

TextBlock::TextBlock()
{
    Color = {0.0f, 0.0f, 0.0f,0.0f};
    RenderMode(Absolute);
    ControlSize = {75.0f, ANA_TEXT_DEFAULT_LINE_HEIGHT};
}

TextBlock::TextBlock(const char* text, glm::vec4 color)
{
    Color = {0.0f, 0.0f, 0.0f,0.0f};
    FontColor = color;
    RenderMode(Absolute);
    ControlSize = {75.0f, ANA_TEXT_DEFAULT_LINE_HEIGHT};
    Text(text);
}

TextBlock::~TextBlock()
{
    auto& textContext = Resources::ResourceManager::GetCurrent()->TextContext;
    textContext.Remove(id);
}

void TextBlock::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    auto size = GetSizeForRender();
    auto offset = GetActualControlOffset();

    this->scale = {size.x(), size.y()};
    this->translation = {offset.x(), offset.y()};
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void TextBlock::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    if (id != uint32_t(-1))
    {
        auto swapChain = SwapChain::GetCurrent();
        auto swapChainExtent = swapChain->GetExtent();
        auto offset = Vec2{renderOffset.x(), renderOffset.y()};
        offset.x() *= float(Extent.width) / float(swapChainExtent.width);
        offset.y() *= float(Extent.height) / float(swapChainExtent.height);
        auto& textContext = Resources::ResourceManager::GetCurrent()->TextContext;
        auto info = textContext.GetInfoById(id);
        info->color = FontColor;

        info->offset = {offset.x(), offset.y()};
        info->size = FontSize * sqrtf(glm::length(glm::vec2(swapChain->ScaleX * 0.7071f, swapChain->ScaleY * 0.7071f)));
        info->offset.y += renderSize.y() - info->size * charSize.y().value * 0.75f / float(Extent.height);
        info->visible = visible;
        info->scissor = {   bounding.x * float(Extent.width),
                            bounding.y * float(Extent.height),
                            bounding.z * float(Extent.width),
                            bounding.w * float(Extent.height)};
        textContext.UpdateLayout(id);
        ControlSize = {FontSize * (charSize.x().value * float(asciiLen) + widthCharSize.x().value * float(wideLen)) * 0.5f,
            FontSize * charSize.y().value};
    }

}

inline void getTextLength(const char* newText, uint32_t len, uint32_t& asciiLen, uint32_t& wideLen)
{
    asciiLen = 0;
    wideLen = 0;
    for (uint32_t i = 0, ch; i < len; i++)
    {
            //assert(ch < char(resourceManager->Characters.size()));
        ch = int(newText[i]);
        if ((ch & 0x80) == 0x00)
        {
            asciiLen++;
            continue;
        }
        else if ((ch & 0xE0) == 0xC0)
        {
            i += 1;
        }
        else if ((ch & 0xF0) == 0xE0)
        {
            i += 2;
        }
        else if ((ch & 0xF8) == 0xF0)
        {
            i += 3;
        }
        wideLen++;
    }
}

void TextBlock::Text(const char* newText)
{
    auto& textContext = Resources::ResourceManager::GetCurrent()->TextContext;
    auto info = textContext.GetInfoById(id);
    if (info)
    {
        textContext.UpdateText(id, newText);
    }
    else
    {
        id = textContext.Insert({0, {0.0f, 0.0f}, FontColor, {}, newText, true, 0});
    }
    getTextLength(newText, strlen(newText), asciiLen, wideLen);
    RequestUpdate();
}

const char* TextBlock::Text()
{
    auto info = Resources::ResourceManager::GetCurrent()->TextContext.GetInfoById(id);
    return info ? info->text.c_str() : "";
}

void TextBlock::Insert(size_t index, uint32_t ch)
{
    auto& textContext = Resources::ResourceManager::GetCurrent()->TextContext;
    auto info = textContext.GetInfoById(id);
    if (info)
    {
        info->text.insert(info->text.begin() + index, ch);
        textContext.UpdateText(id, info->text);
    }
}
