//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/AnA_Button.hpp"
#include "../../Core/Headers/AnA_Object.hpp"
#include "Headers/AnA_Control.hpp"

using namespace AnA;
using namespace AnA::Controls;

AnA_Button::AnA_Button() : AnA_Control()
{
    controlSize = {40.f, 20.f};
    AnA_Object::CreateShape(ANA_CURVED_RECTANGLE, {}, {}, glm::vec3(0.6f, 0.6f, 0.7f), &Properties);
}
