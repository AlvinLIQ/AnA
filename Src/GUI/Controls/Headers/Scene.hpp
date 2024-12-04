#pragma once

#include "Control.hpp"
#include <optional>

namespace AnA
{
    namespace Controls
    {
        class Scene : public Control
        {
        public:
            Scene();
            ~Scene();
            void ImageInfo(VkDescriptorImageInfo _imageInfo)
            {
                imageInfo = _imageInfo;
            }

            virtual void PrepareDraw();
            VkDescriptorImageInfo GetDescriptorImageInfo();
        private:
            std::optional<VkDescriptorImageInfo> imageInfo;
        };
    }
}