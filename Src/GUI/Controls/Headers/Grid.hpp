#pragma once

#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        class Grid : public ItemsPresenter
        {
        public:
            Grid();
            ~Grid();
        protected:
            virtual void prepareDraw();
        };
    }
}