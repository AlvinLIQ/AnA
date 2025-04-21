#include "Headers/ToggleSwitch.hpp"

using namespace AnA;
using namespace Controls;

void ToggleSwitch_Toggled(ToggleSwitch* toggleSwitch, PointerEventArgs& )
{
    toggleSwitch->Toggled = !toggleSwitch->Toggled;
    toggleSwitch->Color = toggleSwitch->Toggled ? 
                    glm::vec4{0.6f, 0.9f, 0.6f, 1.0f} : 
                    glm::vec4{0.9f, 0.3f, 0.3f, 1.0f};

}

ToggleSwitch::ToggleSwitch() : Control()
{
    RenderMode(Auto);
    Color = glm::vec4{0.9f, 0.3f, 0.3f, 1.0f};
    PointerEvents[PointerEventType::Released].push_back(reinterpret_cast<PointerEventHandler>(ToggleSwitch_Toggled));
}