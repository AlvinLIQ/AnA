#include "Headers/ToggleSwitch.hpp"

using namespace AnA;
using namespace Controls;

void ToggleSwitch::ToggleSwitch_Toggled(ToggleSwitch* toggleSwitch, PointerEventArgs& )
{
    bool toggle = toggleSwitch->toggled;
    toggleSwitch->Toggle(!toggle);
    if (toggleSwitch->Toggled)
        toggleSwitch->Toggled(toggle);
}

ToggleSwitch::ToggleSwitch() : Control()
{
    RenderMode(Auto);
    Color = glm::vec4{0.3f, 0.3f, 0.3f, 1.0f};
    PointerEvents[PointerEventType::Released].push_back(reinterpret_cast<PointerEventHandler>(ToggleSwitch_Toggled));
}

void ToggleSwitch::Toggle(bool _toggled)
{
    toggled = _toggled;
    Color = toggled ?
                    glm::vec4{0.9f, 0.9f, 0.9f, 1.0f} :
                    glm::vec4{0.3f, 0.3f, 0.3f, 1.0f};
    RequestUpdate();
}
