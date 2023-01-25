#include "Headers/AnA.h"
#include "Headers/AnA_App.h"

using namespace AnA;

int main()
{
    AnA_App* aApp = new AnA_App;
    aApp->Init();
    aApp->Run();
    delete aApp;
    aApp = nullptr;
    return 0;
}