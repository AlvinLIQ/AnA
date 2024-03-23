//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/Button.hpp"
#include "Headers/Control.hpp"
#include "Styles/Default/ControlStyle.hpp"

using namespace AnA;
using namespace AnA::Controls;

Button::Button() : Control()
{
    SetRenderMode(ButtonRenderMode);
    controlSize = ButtonMinSize;
    Object::CreateShape(ANA_CURVED_RECTANGLE, {}, {}, glm::vec3(0.6f, 0.6f, 0.7f), &Properties);
}
