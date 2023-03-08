#include "Headers/AnA_Object.hpp"

using namespace AnA;

AnA_Object::id_t currentId = 0;

AnA_Object::AnA_Object()
{
    id = currentId++;
}

AnA_Object::~AnA_Object()
{
    if (Model != nullptr)
    {
        Model.reset();
        //Model = nullptr;
    }
}

void AnA_Object::PrepareDraw()
{

}