project('AnA', ['c', 'cpp'],
    default_options: ['c_std=c17', 'cpp_std=c++20', 'warning_level=2'],
    subproject_dir: 'Src/3rdParty/SubProjects')

add_project_arguments(language: 'cpp')
add_project_link_arguments('-fuse-ld=lld', language: 'cpp')
cc = meson.get_compiler('cpp')

vulkan_dep = dependency('vulkan', required: true)  # Vulkan SDK

cflags = []
if get_option('buildtype') != 'debug'
    cflags += ['-DRELEASE_BUILD']
else
    cflags += ['-DDEBUG_BUILD']
endif

cflags += ['-DCULL=' + get_option('cull').to_string()]

conf_data = configuration_data()
conf_data.set('HAVE_GLSLANG', 0)

subdir('Src/3rdParty/')
subdir('Src/Core/')
subdir('Src/GUI')
subdir('Src/Editor/')
subdir('Src/Examples/')

AnA_dep = declare_dependency(
  link_with: [AnACore, AnAControls],
  dependencies: [vulkan_dep, sdl3_dep],
  include_directories: include_directories('Src')
)

meson.override_dependency('AnA', AnA_dep)
