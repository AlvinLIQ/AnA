#pragma once

#include "../Camera/Headers/Camera.hpp"
#include "Object.hpp"
#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "../RenderSystem/Headers/RenderSystem.hpp"
#include "../Input/Headers/InputManager.hpp"
#include <cstdint>
#include <vector>
#include <thread>

namespace AnA
{
    class App
    {
    public:
        App();
        ~App();
        
        App(const App&) = delete;
        App &operator=(const App&) = delete;

        void Init();
        void Run();
        void Cleanup();
        void Exit();

        std::vector<Object*> &GetObjects()
        {
            return objects;
        }

        static std::shared_ptr<Model> &Get2DModel();
        static void CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model);

        Cameras::Camera &GetCamera()
        {
            return camera;
        }

        Input::InputManager *&GetInputManager()
        {
            return aInputManager;
        }

        SwapChain *&GetSwapChain()
        {
            return aRenderer->GetSwapChain();
        }

        Device *&GetDevice()
        {
            return aDevice;
        }
    private:
        Window *aWindow;
        Instance *aInstance;
        Device *aDevice;
        Renderer *aRenderer;
        RenderSystems::RenderSystem *aRenderSystem;
        Input::InputManager *aInputManager;

        std::vector<Object*> objects;

        Cameras::Camera camera;

        std::thread uiThread;
        static void startUILoop(std::thread &loopThread);
        static void waitUILoop(std::thread &loopThread);
        static void uiLoop();
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    };
}
