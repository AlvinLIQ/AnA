//b'\xd5I5f\x9do-\xc2\xeeTU\x08E\xaaM\xd3'
#include "Headers/AnA_Button.h"
#include "../../Core/Headers/AnA_Object.h"

using namespace AnA;
using namespace AnA::Controls;

AnA_Button::AnA_Button() : AnA_Control()
{
    ItemProperties itemProperties;
    AnA_Object::CreateShape(ANA_CURVED_RECTANGLE, {}, {40.f, 20.f}, glm::vec3(0.6f, 0.6f, 0.7f), &itemProperties);
    ItemsProperties.push_back(itemProperties);
}

AnA_Button::~AnA_Button()
{

}