#include "Headers/AnA_Control.h"
#include "../../Core/Headers/AnA_App.h"

using namespace AnA;
using namespace AnA::Controls;

AnA_Control::AnA_Control() : AnA_Object()
{
    this->Model = AnA_App::Get2DModel();
}

AnA_Control::~AnA_Control()
{
    
}