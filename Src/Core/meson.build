subdir('ShaderCodes')

AnACore = static_library('AnACore', ['App.cpp', 'Buffer.cpp', 'Device.cpp', 'Instance.cpp',
    'Pipeline.cpp', 'Renderer.cpp', 'SwapChain.cpp', 'Window.cpp', 'Camera/CameraController.cpp',
    'Physics/Gravity.cpp', 'Resources/Scene.cpp', 'Resources/ShadowMap.cpp',
    'Camera/Camera.cpp', 'Resources/CommandBuffer.cpp', 'Resources/Model.cpp', 'Resources/Shape.cpp',
    'Input/InputManager.cpp', 'Resources/Descriptor.cpp', 'Resources/ResourceManager.cpp', 'Resources/Texture.cpp',
    'Resources/Text.cpp', 'Resources/Meshes.cpp', 'Resources/Animation.cpp',
    'Physics/CollisionDetect.cpp', 'Resources/Lights.cpp', 'Resources/Shader.cpp'],
    include_directories : [vma_include, cdt_include, sdl3_include],
    link_with: [meshoptimizer_lib],
    dependencies: [vulkan_dep, sdl3_dep],
    cpp_args: cflags,
    sources: [shader_hpp])
