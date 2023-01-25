#include "Headers/AnA_Object.h"
#include "Headers/AnA_Model.h"

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
        delete Model;
        Model = nullptr;
    }
}