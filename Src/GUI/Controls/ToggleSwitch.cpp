#include "Headers/ToggleSwitch.hpp"

using namespace AnA;
using namespace Controls;

void Toggled(ToggleSwitch* toggleSwitch, PointerEventArgs& )
{
    toggleSwitch->Toggled = !toggleSwitch->Toggled;
    toggleSwitch->Color = toggleSwitch->Toggled ? 
                    glm::vec4{0.6f, 0.9f, 0.6f, 1.0f} : 
                    glm::vec4{0.9f, 0.6f, 0.6f, 1.0f};

}

ToggleSwitch::ToggleSwitch()
{
    Color = glm::vec4{0.9f, 0.6f, 0.6f, 1.0f};
    PointerEvents[PointerEventType::Released].emplace_back(reinterpret_cast<PointerEventHandler>(Toggled));
}