#pragma once
#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        enum Orientations {Horizontal, Vertical};
        class StackPanel : public ItemsPresenter
        {
        public:
            StackPanel();
            ~StackPanel();

            void PrepareDraw();
            Orientations Orientation {Horizontal};
        };
    }
}