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
        enum Orientations {Horizontal = 1, Vertical = 0};
        class Control : public AnA::ShapeInfo
        {
        public:
            Control();
            virtual ~Control();
            
            AlignmentType HorizontalAlignment {ControlHorizontalAlignment};
            AlignmentType VerticalAlignment {ControlVerticalAlignment};

            POS_2F ControlOffset {};
            POS_2F GetActualControlOffset();

            SIZE_2F ControlSize {AnA::ControlSize};
            SIZE_2F GetSizeForRender();
            void SizeRequest(SIZE_2F newSize)
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
            bool ProcessEventArgs(PointerEventArgs& args, PointerEventType& actualEventType);
            virtual void PointerEventTrigger(PointerEventArgs& args);
            static void InitControl(SwapChain* swapChain);
            static float* GetScale();
            static VkExtent2D GetSwapChainExtent();
            static Device* GetDevice();
            static void GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles);

            void RenderOffset(POS_2F newOffset)
            {
                renderOffset = newOffset;
            }
            POS_2F RenderOffset()
            {
                return renderOffset;
            }
            void RenderSize(SIZE_2F newSize)
            {
                renderSize = newSize;
            }
            SIZE_2F RenderSize()
            {
                return renderSize;
            }

            std::vector<PointerEventHandler> PointerEvents[PointerEventType::Moving + 1] = {};

            bool IsFocused();
            void Focus();
            void Unfocus();
            bool IsPressed() const
            {
                return pressed;
            }
            static void ClearFocus();
            static Control* GetFocused();
            
            bool IsInside(CursorPosition pos);
            static bool IsInside(CursorPosition& pos, POS_2F& offset, SIZE_2F& size);

            virtual VkDescriptorImageInfo GetDescriptorImageInfo();
            virtual void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);
        private:
            AlignType renderMode {ControlRenderMode};
            POS_2F renderOffset{};
            SIZE_2F renderSize{};
            bool cursorInside = false;
            bool pressed = false;
        protected:
            SIZE_2F minSize {ControlMinSize};
            SIZE_2F maxSize {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        };
    }
}