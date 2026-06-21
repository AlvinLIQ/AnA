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
        typedef void(*FloatValueCallback)(float);
        enum Orientations {Horizontal = 1, Vertical = 0};
        class Control : public AnA::ShapeInfo
        {
        public:
            Control();
            virtual ~Control();

            AlignmentType HorizontalAlignment {ControlHorizontalAlignment};
            AlignmentType VerticalAlignment {ControlVerticalAlignment};

            Vec2 ControlOffset {};
            Vec2 GetActualControlOffset();

            Vec2 ControlSize {AnA::ControlSize};
            virtual Vec2 GetSizeForRender();
            void SizeRequest(Vec2 newSize)
            {
                if (newSize.x() < minSize.x() || newSize.y() < minSize.y() ||
                    newSize.x() > maxSize.x() || newSize.y() > maxSize.y())
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
            virtual void CharacterRecevied(uint32_t ch);
            static void InitControl(SwapChain* swapChain);
            static Float* GetScale();
            static VkExtent2D GetSwapChainExtent();
            static Device* GetDevice();
            static void GetInputProfile(Control* mainControl, std::vector<Input::InputProfile>& profiles);
            static CursorPosition GetRelativePosition(const CursorPosition& pos, const VkExtent2D& extent);

            void RenderOffset(Vec2 newOffset)
            {
                renderOffset = newOffset;
            }
            Vec2& RenderOffset()
            {
                return renderOffset;
            }
            void RenderSize(Vec2 newSize)
            {
                renderSize = newSize;
            }
            Vec2& RenderSize()
            {
                return renderSize;
            }
            Vec2 ActualRenderOffset()
            {
                return renderOffset;//{renderOffset.x() * 0.5f + 0.5f - renderSize.x() * 0.5f, renderOffset.y() * 0.5f + 0.5f - renderSize.y() * 0.5f};
            }
            virtual Vec2 ContentRenderSize()
            {
                return RenderSize();
            }

            std::vector<PointerEventHandler> PointerEvents[PointerEventType::Scrolled + 1] = {};

            bool IsFocused();
            void Focus();
            void Unfocus();
            short FocusType = 0;
            bool IsPressed() const
            {
                return pressed;
            }
            static void ClearFocus();
            static Control* GetFocused();

            Control* Parent{nullptr};

            bool IsCursorInside() const
            {
                return cursorInside;
            }
            bool IsInside(CursorPosition pos);
            static bool IsInside(CursorPosition& pos, Vec2& offset, Vec2& size);

            static bool NeedUpdate();
            static bool BeginUpdate();
            static void RequestUpdate();
            static void EndUpdate();

            static bool TextLayoutNeedReset();
            static void RequestTextLayoutReset();
            static void EndTextLayoutReset();

            void Texture(const std::string path);

            virtual VkDescriptorImageInfo GetDescriptorImageInfo();
            virtual void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);
        private:
            AlignType renderMode {ControlRenderMode};
            bool cursorInside = false;
            bool pressed = false;
        protected:
            Vec2 renderOffset{};
            Vec2 renderSize{};
            Vec2 minSize {ControlMinSize};
            Vec2 maxSize {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        };
    }
}
