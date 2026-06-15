#include "Headers/TextBlock.hpp"
#include "../../Core/Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace AnA::Controls;

const Vec2 charSize = {0.836363613f, 0.99999994f};

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

    this->Transform.scale = {size.x(), size.y(), 1.f};
    this->Transform.translation = {offset.x(), offset.y(), 0.f};
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void TextBlock::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
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
        info->visible = true;
        textContext.UpdateLayout(id);
        ControlSize = {FontSize * charSize.x().value * float(info->length * 0.5f),
            FontSize * charSize.y().value};
    }

    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
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
