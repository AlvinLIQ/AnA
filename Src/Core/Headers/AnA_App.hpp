#pragma once

#include "AnA_Camera.hpp"
#include "AnA_Object.hpp"
#include "AnA_Renderer.hpp"
#include "AnA_Window.hpp"
#include "AnA_Instance.hpp"
#include "AnA_Device.hpp"
#include "../RenderSystem/Headers/AnA_RenderSystem.hpp"
#include <cstdint>
#include <vector>
#include <thread>

namespace AnA
{
    class AnA_App
    {
    public:
        AnA_App();
        ~AnA_App();
        
        AnA_App(const AnA_App&) = delete;
        AnA_App &operator=(const AnA_App&) = delete;

        void Init();
        void Run();
        void Cleanup();
        void Exit();

        std::vector<AnA_Object*> &GetObjects()
        {
            return objects;
        }

        static AnA_Model* Get2DModel();
        static void CreateModel(const AnA_Model::ModelInfo &modelInfo, AnA_Model** pModel);

        AnA_Camera &GetCamera()
        {
            return camera;
        }

    private:
        AnA_Window* aWindow;
        AnA_Instance* aInstance;
        AnA_Device* aDevice;
        AnA_Renderer* aRenderer;
        RenderSystems::AnA_RenderSystem* aRenderSystem;

        std::vector<AnA_Object*> objects;
        void loadObjects();

        AnA_Camera camera;

        std::thread uiThread;
        static void startUILoop(std::thread &loopThread);
        static void waitUILoop(std::thread &loopThread);
        static void uiLoop();
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    };
}
