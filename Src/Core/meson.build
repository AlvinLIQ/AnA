subdir('ShaderCodes')

AnACore = static_library('AnACore', ['App.cpp', 'Buffer.cpp', 'Device.cpp', 'Instance.cpp', 
    'Pipeline.cpp', 'Renderer.cpp', 'SwapChain.cpp', 'Window.cpp', 'Camera/CameraController.cpp',
    'Physics/Gravity.cpp', 'Resources/Mesh.cpp', 'Resources/ShadowMap.cpp', 'Systems/ShadowSystem.cpp',
    'Camera/Camera.cpp', 'Resources/CommandBuffer.cpp', 'Resources/Model.cpp', 'Resources/Shape.cpp',
    'Input/InputManager.cpp', 'Resources/Descriptor.cpp', 'Resources/ResourceManager.cpp', 'Resources/Texture.cpp',
    'Physics/CollisionDetect.cpp', 'Resources/Lights.cpp', 'Resources/Shader.cpp', 'Systems/RenderSystem.cpp'],
    cpp_args: ['-std=c++23'],
    link_with: [meshoptimizer_lib],
    dependencies: [vulkan_dep, glfw_dep],
    sources: [shader_hpp])