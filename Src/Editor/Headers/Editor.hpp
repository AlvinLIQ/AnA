#pragma once
#include "../../Core/Headers/App.hpp"

namespace AnA
{
    namespace Editors
    {
        class Editor : public App
        {
        public:
            Editor();
            ~Editor();
        protected:
            void onCommandBufferRecording(VkCommandBuffer& commandBuffer); 
        };
    }
}