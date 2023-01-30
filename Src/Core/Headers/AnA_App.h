#pragma once

#include "AnA_Object.h"
#include "AnA_Renderer.h"
#include "AnA_Window.h"
#include "AnA_Instance.h"
#include "AnA_Device.h"
#include "../RenderSystem/Headers/AnA_RenderSystem.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace AnA
{
    class AnA_App
    {
    public:
        AnA_App();
        ~AnA_App();

        void Init();
        void Run();
        void Cleanup();
        void Exit();

    private:
        AnA_Window* aWindow;
        AnA_Instance* aInstance;
        AnA_Device* aDevice;
        AnA_Renderer* aRenderer;
        RenderSystems::AnA_RenderSystem* aRenderSystem;

        std::vector<AnA_Object*> objects;
        void loadObjects();
    };
}
