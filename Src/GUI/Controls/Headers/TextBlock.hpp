#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class TextBlock : public Control
        {
        public:
            TextBlock();
            ~TextBlock();

            virtual void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);

            void Text(const char* newText);
            const char* Text();

            VkDescriptorImageInfo GetDescriptorImageInfo();
            bool IsWrapping;
        protected:
            std::string text = "";
            Texture* texture{nullptr};
        };
    }
}