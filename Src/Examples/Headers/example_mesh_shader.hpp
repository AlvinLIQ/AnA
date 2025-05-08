#pragma once
#include "../../Core/Headers/App.hpp"
#include "../../GUI/Controls/Headers/Slider.hpp"
#include "../../GUI/Controls/Headers/ToggleSwitch.hpp"

namespace AnA
{
    namespace Examples
    {
        class example_mesh_shader : public App
        {
        public:
            example_mesh_shader() : App()
            {
                aResourceManager.MainCameraInfo.far = 100.0f;
                aResourceManager.GlobalLight.Color = glm::vec3{0.8f};
            }
            ~example_mesh_shader()
            {

            }
            void Init()
            {
                loopCallback = example_mesh_shader::onLoop;
                App::Init();
                aResourceManager.MainControl = InitControl();
                sceneOffset.x = 350;
                aInputManager.GlobalProfile.flag = Input::InputProfileFlags::None;
                Input::KeyMapConfig keyMapConfig;
                keyMapConfig.keyCode = GLFW_KEY_TAB;
                keyMapConfig.param = &aInputManager;
                keyMapConfig.callBack = [](void* param)
                {
                    auto aInputManager = reinterpret_cast<Input::InputManager*>(param);
                    /*
                    auto& activeProfile = aInputManager->GetActiveProfile();
                    aInputManager->ProcessProfileFlag((activeProfile.flag ^= Input::Disabled) & Input::Disabled ?
                        aInputManager->GlobalProfile.flag :
                        activeProfile.flag);*/
                    if (&aInputManager->GetActiveProfile() == aInputManager->GetProfiles().data())
                    {
                        aInputManager->SetActiveProfile(1);
                    }
                    else
                    {
                        aInputManager->SetActiveProfile(0);
                    }
                };
                aInputManager.GlobalProfile.keyMapConfigs.push_back(keyMapConfig);
                Controls::Control::GetInputProfile(aResourceManager.MainControl, aInputManager.GetProfiles());
            }
            Controls::Control* InitControl();
        private:
            std::unordered_map<std::string, Controls::Control*> controlMap;
        protected:
            static void onLoop()
            {
                auto app = reinterpret_cast<example_mesh_shader*>(App::GetCurrent());
                app->terrainPushConstants.density = (reinterpret_cast<Controls::Slider*>(app->controlMap["densitySlider"])->Value);
                app->terrainPushConstants.height = (reinterpret_cast<Controls::Slider*>(app->controlMap["heightSlider"])->Value + 0.01f) * 10.0f;
                app->terrainPushConstants.texture = reinterpret_cast<Controls::ToggleSwitch*>(app->controlMap["textureSwitch"])->Toggle() ? 0 : 1;
                app->terrainPushConstants.calNormal = reinterpret_cast<Controls::ToggleSwitch*>(app->controlMap["lightSwitch"])->Toggle() ? 1 : 0;
            }
        };
    }
}