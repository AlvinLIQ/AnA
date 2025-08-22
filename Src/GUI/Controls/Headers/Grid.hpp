#pragma once

#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        struct GridCell
        {
            short state; //0 Max 1 Min 2 Manual
            float offset;
		    float size;
        };

        class Grid : public ItemsPresenter
        {
        public:
            Grid();
            ~Grid();
            virtual void Child(Control* newItem, uint32_t x = 0, uint32_t y = 0) override;
            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
            std::vector<GridCell> cells[2];
            std::vector<std::vector<uint32_t>> itemsPositions;
        };
    }
}