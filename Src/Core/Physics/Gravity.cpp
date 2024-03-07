#include "Headers/Gravity.hpp"

using namespace AnA;
using namespace Physics;

Gravity::Gravity()
{

}

void Gravity::Drop(float sTime)
{
    startTime = sTime;
}

void Gravity::StopDropping()
{
    startTime = 0.0f;
}

float Gravity::GetDropDistance(float curTime)
{
    float dTime = curTime - startTime;
    return dTime * dTime * G * 0.5f;
}