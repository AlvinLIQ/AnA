subdir('ShaderCodes')

AnACore = static_library('AnACore', ['App.cpp', 'Buffer.cpp', 'Device.cpp', 'Instance.cpp',
    'Pipeline.cpp', 'Renderer.cpp', 'SwapChain.cpp', 'Window.cpp', 'Camera/CameraController.cpp',
    'Physics/Gravity.cpp', 'Resources/Scene.cpp', 'Resources/ShadowMap.cpp', 'Systems/ShadowSystem.cpp',
    'Camera/Camera.cpp', 'Resources/CommandBuffer.cpp', 'Resources/Model.cpp', 'Resources/Shape.cpp',
    'Input/InputManager.cpp', 'Resources/Descriptor.cpp', 'Resources/ResourceManager.cpp', 'Resources/Texture.cpp',
    'Resources/Text.cpp',
    'Physics/CollisionDetect.cpp', 'Resources/Lights.cpp', 'Resources/Shader.cpp', 'Systems/RenderSystem.cpp'],
    cpp_args: ['-std=c++23'],
    include_directories : [vma_include, cdt_include],
    link_with: [meshoptimizer_lib],
    dependencies: [vulkan_dep, glfw_dep],
    sources: [shader_hpp])
