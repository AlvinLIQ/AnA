#pragma once
#include "../../../Core/Headers/SwapChain.hpp"
#include "Types.hpp"
#include "../Styles/Default/ControlStyle.hpp"
#include <limits>

namespace AnA
{
    namespace Controls
    {
        class Control
        {
        public:
            Control();
            
            AlignmentType HorizontalAlignment {ControlHorizontalAlignment};
            AlignmentType VerticalAlignment {ControlVerticalAlignment};

            POS_F ControlOffset {};
            POS_F GetActualControlOffset(SIZE_F renderSize)
            {
                POS_F actualOffset;
                float* pOffset = (float*)&actualOffset;
                float* pSize = (float*)&renderSize;
                AlignmentType Alignments[]{HorizontalAlignment, VerticalAlignment};
                for (int i = 0; i < 2; i++)
                {
                    if (Alignments[i] == AlignmentType::Center)
                        pOffset[i] = 0.f;
                    else if (Alignments[i] == AlignmentType::Start)
                        pOffset[i] = pSize[i] / 2.f - 1.0f;
                    else
                        pOffset[i] = 1.0f - pSize[i] / 2.f;
                }
                
                actualOffset.x += ControlOffset.x;
                actualOffset.y += ControlOffset.y;
                return actualOffset;
            }

            SIZE_F ControlSize {AnA::ControlSize};
            SIZE_F GetSizeForRender() const
            {
                SIZE_F renderSize;
                if (renderMode == AlignType::Relative)
                {
                    auto extent = GetSwapChainExtent();
                    renderSize.Width = ControlSize.Width / (float)extent.height;
                    renderSize.Height = ControlSize.Height / (float)extent.height;
                }
                else if (renderMode == AlignType::Absolute)
                {
                    auto extent = GetSwapChainExtent();
                    renderSize.Width = ControlSize.Width / (float)extent.width;
                    renderSize.Height = ControlSize.Height / (float)extent.height;
                }
                else if (renderMode == AlignType::Auto)
                {
                    auto scale = ControlSize;
                    renderSize = ControlSize;
                }
                return renderSize;
            }
            void SetSizeRequest(SIZE_F newSize)
            {
                if (newSize.Width < minSize.Width || newSize.Height < minSize.Height || 
                    newSize.Width > maxSize.Width || newSize.Height > maxSize.Height)
                    return;

                ControlSize = newSize;
            }

            AlignType GetRenderMode() const
            {
                return renderMode;
            }
            void SetRenderMode(AlignType newRenderMode)
            {
                if (newRenderMode == renderMode)
                    return;
                //
                renderMode = newRenderMode;
            }

            virtual void PrepareDraw();
            virtual void Draw(VkCommandBuffer commandBuffer);
            static void InitControl(SwapChain* swapChain);
            static VkExtent2D GetSwapChainExtent();
            static Device& GetDevice();

            std::vector<PointerEventHandler> PointerEvents[PointerEventType::Moving + 1];
            void PointerEventTrigger(PointerEventArgs& args)
            {
                for (auto event : PointerEvents[args.EventType])
                {
                    event(this, args);
                }
            }

            bool IsFocused();
            void Focus();
            void Unfocus();
            
            bool IsInside(POS_F pos);

            glm::vec3 Color{1.0f};
        private:
            AlignType renderMode {ControlRenderMode};
        protected:
            SIZE_F minSize {ControlMinSize};
            SIZE_F maxSize {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        };
    }
}