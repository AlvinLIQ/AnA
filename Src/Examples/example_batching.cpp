#define ANA_INCLUDE_CONTROL
#include "../Core/Headers/App.hpp"

using namespace AnA;

int main()
{
    App app{};
    app.Init();
    
    app.Run();
    return 0;
}