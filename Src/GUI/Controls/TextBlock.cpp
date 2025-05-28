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
    auto& textContext = Resource::ResourceManager::GetCurrent()->TextContext;
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

void TextBlock::ApplyRenderInfo(Shape* , std::vector<VkDescriptorImageInfo>& , uint32_t& )
{
    if (id != -1u)
    {
        auto swapChain = SwapChain::GetCurrent();
        auto swapChainExtent = swapChain->GetExtent();
        auto renderSize = RenderSize();
        auto renderOffset = RenderOffset();
        auto offset = Vec2{renderOffset.x() - renderSize.x() + 1.0f, renderOffset.y() - renderSize.y() * 0.5f};
        offset.x() *= float(Extent.width) / float(swapChainExtent.width);
        offset.y() *= float(Extent.height) / float(swapChainExtent.height);
        auto& textContext = Resource::ResourceManager::GetCurrent()->TextContext;
        auto info = textContext.GetInfoById(id);
        info->color = FontColor;

        info->offset = {offset.x() * 0.5f, offset.y() * 0.5f + 0.5f};
        textContext.UpdateLayout(id);
    }

    //Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void TextBlock::Text(const char* newText)
{
    auto& textContext = Resource::ResourceManager::GetCurrent()->TextContext;
    auto info = textContext.GetInfoById(id);
    if (info)
    {
        textContext.UpdateText(id, newText);
    }
    else
    {
        id = textContext.Insert({FontSize, {0.0f, 0.0f}, FontColor, newText});
    }
    ControlSize = {FontSize * charSize.x() * float(strlen(newText) * 0.5f), FontSize * charSize.y()};
}

const char* TextBlock::Text()
{
    auto info = Resource::ResourceManager::GetCurrent()->TextContext.GetInfoById(id);
    return info ? info->text.c_str() : "";
}
