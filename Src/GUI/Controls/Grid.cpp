#include "Headers/Grid.hpp"

using namespace AnA;
using namespace Controls;

Grid::Grid()
{

}

Grid::~Grid()
{

}

void Grid::Child(Control* newItem, uint32_t x, uint32_t y)
{
    while (x >= cells[0].size())
        cells[0].push_back({});
    while (y >= cells[1].size())
        cells[1].push_back({});
    while (y >= itemsPositions.size())
        itemsPositions.push_back({});
    while (x >= itemsPositions[y].size())
        itemsPositions[y].push_back({});
    items.push_back(newItem);
    itemsPositions[y][x] = uint32_t(items.size() - 1);
    RequestUpdate();
}

void Grid::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void Grid::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    auto renderSize = GetSizeForRender();
    auto renderOffset = GetActualControlOffset();
    float maxSize, offset;
    for (uint32_t i = 0, j, maxCells = 0; i < 2; i++)
    {
        maxSize = renderSize[i];
        for (j = 0; j < uint32_t(cells[i].size()); j++)
        {
            auto& cell = cells[i][j];
            if (cell.state)
                maxSize -= cell.size;
            else
                maxCells++;
        }
        maxSize /= float(maxCells);
        offset = renderOffset[i];
        for (j = 0; j < uint32_t(cells[i].size()); j++)
        {
            auto& cell = cells[i][j];
            if (!cell.state)
            {
                cell.size = maxSize;
            }
            cell.offset = offset;

            offset += cell.size;
        }
    }
    for (uint32_t i = 0, j; i < uint32_t(itemsPositions.size()); i++)
    {
        for (j = 0; j < uint32_t(itemsPositions[i].size()); j++)
        {
            auto pos = itemsPositions[i][j];
            auto& cellX = cells[0][i], &cellY = cells[1][j];
            items[pos]->RenderOffset()[0] = cellX.offset;
            items[pos]->RenderOffset()[1] = cellY.offset;
            items[pos]->RenderSize()[0] = cellX.size;
            items[pos]->RenderSize()[1] = cellY.size;
            items[pos]->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
        }
    }
}