project('AnA', ['c', 'cpp'],
    default_options: ['c_std=c17', 'cpp_std=c++20', 'warning_level=2'])

add_project_arguments(language: 'cpp')
add_project_link_arguments('-fuse-ld=lld', language: 'cpp')
vulkan_dep = dependency('vulkan', required: true)  # Vulkan SDK

cflags = []
if get_option('buildtype') == 'release'
    cflags += ['-DRELEASE_BUILD']
else
    cflags += ['-DDEBUG_BUILD']
endif

if host_machine.system() == 'windows'
    glfw_dep = declare_dependency(
        include_directories: include_directories('C:/glfw3/include'),
        link_args: ['C:/glfw3/lib/glfw3.lib']
    )
else
    glfw_dep = dependency('glfw3', required: true)      # GLFW
endif

glslang_dep = dependency('glslang', required: false)
conf_data = configuration_data()
if glslang_dep.found()
  conf_data.set('HAVE_GLSLANG', 1)
else
  conf_data.set('HAVE_GLSLANG', 0)
endif

subdir('Src/3rdParty/')
subdir('Src/Core/')
subdir('Src/GUI')
subdir('Src/Editor/')
subdir('Src/Examples/')
