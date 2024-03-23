#pragma once

#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "../Systems/Headers/RenderSystem.hpp"
#include "../Systems/Headers/ShadowSystem.hpp"
#include "../Input/Headers/InputManager.hpp"
#include "../Resources/Headers/Object.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
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

        Objects SceneObjects;

        static void CreateCubeModel(std::shared_ptr<Model>& model);
        static std::shared_ptr<Model> &Get2DModel();
        static void CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model);

        Input::InputManager& GetInputManager()
        {
            return *aInputManager;
        }

        SwapChain& GetSwapChain()
        {
            return aRenderer->GetSwapChain();
        }

        Device& GetDevice()
        {
            return *aDevice;
        }
    private:
        Window* aWindow;
        Instance* aInstance;
        Device* aDevice;
        Renderer* aRenderer;
        Systems::RenderSystem* aRenderSystem;
        Systems::ShadowSystem* aShadowSystem;
        Input::InputManager* aInputManager;

        Resource::ResourceManager* aResourceManager;

        std::thread uiThread;
        static void startUILoop(std::thread &loopThread);
        static void waitUILoop(std::thread &loopThread);
        static void uiLoop();
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    };
}
