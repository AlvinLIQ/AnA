#pragma once

#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "../Systems/Headers/RenderSystem.hpp"
#include "../Systems/Headers/ShadowSystem.hpp"
#include "../Input/Headers/InputManager.hpp"
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

        static void CreateCubeModel(std::shared_ptr<Model>& model);
        static std::shared_ptr<Model> &Get2DModel();
        static void CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model);

        static App* GetCurrent();
        Input::InputManager& GetInputManager()
        {
            return aInputManager;
        }

        SwapChain& GetSwapChain()
        {
            return aRenderer.GetSwapChain();
        }
        Renderer& GetRenderer()
        {
            return aRenderer;
        }

        Device* GetDevice()
        {
            return &aDevice;
        }
        VkOffset2D &GetSceneOffset()
        {
            actualSceneOffset = {static_cast<int32_t>(sceneOffset.x * aRenderer.GetSwapChain().ScaleX), 
                static_cast<int32_t>(sceneOffset.y * aRenderer.GetSwapChain().ScaleY)};
            return actualSceneOffset;
        }
    private:
        std::thread uiThread;
        static void startUILoop(std::thread &loopThread);
        static void waitUILoop(std::thread &loopThread);
        static void uiLoop();
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        bool commandBufferNeedUpdate = false;
        VkOffset2D actualSceneOffset{};
    protected:
        virtual void createRecordCallBacks();
        virtual void onCommandBufferRecording(VkCommandBuffer& commandBuffer);
        virtual void onLoop();
        Window aWindow;
        Instance aInstance;
        Input::InputManager aInputManager;
        Device aDevice;
        Renderer aRenderer;
        Systems::RenderSystem aRenderSystem;
        Systems::ShadowSystem aShadowSystem;
        Resource::ResourceManager aResourceManager;
        VkOffset2D sceneOffset{};
    };
}
