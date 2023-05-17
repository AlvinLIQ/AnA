//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/Button.hpp"
#include "../../Core/Headers/Object.hpp"
#include "Headers/Control.hpp"

using namespace AnA;
using namespace AnA::Controls;

Button::Button() : Control()
{
    controlSize = {40.f, 20.f};
    Object::CreateShape(ANA_CURVED_RECTANGLE, {}, {}, glm::vec3(0.6f, 0.6f, 0.7f), &Properties);
}
