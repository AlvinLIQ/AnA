#define ANA_INCLUDE_CONTROL
#include "../Core/Headers/App.hpp"
#include "../GUI/Controls/Headers/StackPanel.hpp"

using namespace AnA;

int main()
{
    App app{};
    app.Init();
    Controls::StackPanel* mainControl = new Controls::StackPanel();
    

    Resource::ResourceManager::GetCurrent()->MainControl = mainControl;
    app.Run();
    return 0;
}