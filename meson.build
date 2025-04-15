project('AnA', ['c', 'cpp'],
    default_options: ['c_std=c23', 'cpp_std=c++23'])

add_project_arguments('-Wall', '-Wextra', '-Wpedantic', language: 'cpp')
add_project_link_arguments('-fuse-ld=lld', language: 'cpp')
vulkan_dep = dependency('vulkan', required: true)  # Vulkan SDK
if host_machine.system() == 'windows'
    glfw_dep = declare_dependency(
        include_directories: include_directories('C:/glfw3/include'),
        link_args: ['C:/glfw3/lib/glfw3.lib']
    )
else
    glfw_dep = dependency('glfw3', required: true)      # GLFW
endif

subdir('Src/3rdParty/')
subdir('Src/Core/')
subdir('Src/GUI')
subdir('Src/Editor/')