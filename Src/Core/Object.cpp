#include "Headers/Object.hpp"

using namespace AnA;

Object::id_t currentId = 0;

Object::Object()
{
    id = currentId++;
}

Object::~Object()
{
    if (Model != nullptr)
    {
        Model.reset();
        //Model = nullptr;
    }
}

void Object::PrepareDraw()
{

}

void Object::createDescriptorPool()
{
    
}

void Object::createDescriptorSet()
{
    
}