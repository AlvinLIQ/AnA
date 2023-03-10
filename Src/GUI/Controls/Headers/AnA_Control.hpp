#pragma once
#include "../../../Core/Headers/AnA_Object.hpp"
#include "../../../Core/Headers/AnA_SwapChain.hpp"
#include <limits>

namespace AnA
{
    namespace Controls
    {
        enum AlignType
        {
            Absolute, Relative, Auto
        };
        enum AlignmentType
        {
            Start, Center, End
        };

        enum PointerEventType
        {
            Pressed, Entered, Released, Exited, Moved, Moving
        };
        enum PointerTriggerType
        {
            Touch, Mouse
        };
        struct PointerEventArgs
        {
            uint32_t Position[2];
            PointerEventType EventType;
            PointerTriggerType TriggerType;
        };
        //typedef void (*)(AnA_Control* control) PointerEventHandler;
        struct ANA_SIZE_F
        {
            float Width;
            float Height;
        };
        struct ANA_POS_F
        {
            float x;
            float y;
        };


        class AnA_Control : public AnA_Object
        {
        public:
            AnA_Control();
            
            AlignmentType HorizontalAlignment {Center};
            AlignmentType VerticalAlignment {Center};

            ANA_POS_F ControlOffset {};
            ANA_POS_F GetActualControlOffset(ANA_SIZE_F renderSize)
            {
                ANA_POS_F actualOffset;
                float *pOffset = (float*)&actualOffset;
                float *pSize = (float*)&renderSize;
                AlignmentType Alignments[]{HorizontalAlignment, VerticalAlignment};
                for (int i = 0; i < 2; i++)
                {
                    if (Alignments[i] == AlignmentType::Center)
                        pOffset[i] = 0.f;
                    else if (Alignments[i] == AlignmentType::Start)
                        pOffset[i] = 0.5f - pSize[i] / 2.f;
                    else
                        pOffset[i] = 0.5f - pSize[i] / 2.f;
                }
                
                actualOffset.x += ControlOffset.x;
                actualOffset.y += ControlOffset.y;
                return actualOffset;
            }

            ANA_SIZE_F GetSizeForRender() const
            {
                ANA_SIZE_F renderSize;
                if (renderMode == AlignType::Relative)
                {
                    auto extent = GetSwapChainExtent();
                    renderSize.Width = controlSize.Width / (float)extent.width * (float)extent.height;
                }
                else if (renderMode == AlignType::Absolute)
                {
                    auto extent = GetSwapChainExtent();
                    renderSize.Width = controlSize.Width / (float)extent.width;
                    renderSize.Height = controlSize.Height / (float)extent.height;
                }
                else if (renderMode == AlignType::Auto)
                {
                    auto scale = Properties.transform.scale;
                    renderSize = {scale.x, scale.y};
                }
                return renderSize;
            }
            void SetSizeRequest(ANA_SIZE_F newSize)
            {
                if (newSize.Width < minSize.Width || newSize.Height < minSize.Height || 
                    newSize.Width > maxSize.Width || newSize.Height > maxSize.Height)
                    return;

                controlSize = newSize;
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
            static void InitControl(AnA_SwapChain *swapChain);
            static VkExtent2D GetSwapChainExtent();

            bool IsFocused();
            void Focus();
            void Unfocus();
            
        private:
            AlignType renderMode {Absolute};
        protected:
            ANA_SIZE_F controlSize {};
            ANA_SIZE_F minSize {};
            ANA_SIZE_F maxSize {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        };
    }
}