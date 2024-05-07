#pragma once
#include "../../../Core/Headers/SwapChain.hpp"
#include "Types.hpp"
#include "../Styles/Default/ControlStyle.hpp"
#include "../../../Core/Resources/Headers/Shape.hpp"
#include "../../../Core/Input/Headers/InputManager.hpp"
#include <limits>

namespace AnA
{
    namespace Controls
    {
        class Control : public AnA::ShapeInfo
        {
        public:
            Control();
            virtual ~Control();
            
            AlignmentType HorizontalAlignment {ControlHorizontalAlignment};
            AlignmentType VerticalAlignment {ControlVerticalAlignment};

            POS_F ControlOffset {};
            POS_F GetActualControlOffset(SIZE_F renderSize)
            {
                float* pOffset = (float*)&renderOffset;
                float* pSize = (float*)&this->renderSize;
                AlignmentType Alignments[]{HorizontalAlignment, VerticalAlignment};
                for (int i = 0; i < 2; i++)
                {
                    if (Alignments[i] == AlignmentType::Start)
                        pOffset[i] = pSize[i] / 2.f - 1.0f;
                    else if (Alignments[i] == AlignmentType::End)
                        pOffset[i] = 1.0f - pSize[i] / 2.f;
                    else
                    {
                        pOffset[i] = 0.f;
                        if (Alignments[i] == AlignmentType::Stretch)
                            pSize[i] = 1.f;
                    }
                }
                
                renderOffset.x += ControlOffset.x;
                renderOffset.y += ControlOffset.y;
                return renderOffset;
            }

            SIZE_F ControlSize {AnA::ControlSize};
            SIZE_F GetSizeForRender()
            {
                if (renderMode == AlignType::Relative)
                {
                    renderSize.Width = ControlSize.Width / (float)Extent.width;
                    renderSize.Height = ControlSize.Height / (float)Extent.height;
                }
                else if (renderMode == AlignType::Absolute)
                {
                    renderSize = ControlSize;
                }
                else if (renderMode == AlignType::Auto)
                {
                    renderSize = {ControlSize.Width / Aspect, ControlSize.Height};
                }
                return renderSize;
            }
            void SizeRequest(SIZE_F newSize)
            {
                if (newSize.Width < minSize.Width || newSize.Height < minSize.Height || 
                    newSize.Width > maxSize.Width || newSize.Height > maxSize.Height)
                    return;

                ControlSize = newSize;
            }

            AlignType RenderMode() const
            {
                return renderMode;
            }
            void RenderMode(AlignType newRenderMode)
            {
                if (newRenderMode == renderMode)
                    return;
                //
                renderMode = newRenderMode;
            }

            float Aspect = 1.0f;
            VkExtent2D Extent;

            virtual void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);
            virtual void PointerEventTrigger(PointerEventArgs& args);
            static void InitControl(SwapChain* swapChain);
            static VkExtent2D GetSwapChainExtent();
            static Device* GetDevice();
            static void GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles);

            void RenderOffset(POS_F newOffset)
            {
                renderOffset = newOffset;
            }
            POS_F RenderOffset()
            {
                return renderOffset;
            }
            void RenderSize(SIZE_F newSize)
            {
                renderSize = newSize;
            }
            SIZE_F RenderSize()
            {
                return renderSize;
            }

            std::vector<PointerEventHandler> PointerEvents[PointerEventType::Moving + 1];

            bool IsFocused();
            void Focus();
            void Unfocus();
            
            bool IsInside(CursorPosition pos);
            static bool IsInside(CursorPosition& pos, POS_F& offset, SIZE_F& size);

            virtual VkDescriptorImageInfo GetDescriptorImageInfo();
            virtual void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfoss, uint32_t& shapeCount);
        private:
            AlignType renderMode {ControlRenderMode};
            POS_F renderOffset{};
            SIZE_F renderSize{};
            bool cursorInside = false;
            bool pressed = false;
        protected:
            SIZE_F minSize {ControlMinSize};
            SIZE_F maxSize {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        };
    }
}