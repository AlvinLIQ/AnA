#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class Scene : public Control
        {
        public:
            Scene();
            ~Scene();

            virtual void PrepareDraw();
            
        };
    }
}