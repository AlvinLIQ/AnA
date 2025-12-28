example_mesh_shader_ui = custom_target('Convert UI to cpp code',
  output: 'example_mesh_shader.g.cpp',
  command: [ui_parser, join_paths(meson.project_source_root(), 'Src/Examples/example_mesh_shader.ui'), '@OUTPUT@'],
  depends: [ui_parser], depend_files: ['example_mesh_shader.ui']
  )
example_mesh_shader = executable('example_mesh_shader', [example_mesh_shader_ui , 'example_mesh_shader.cpp'],
    link_with: [AnACore, AnAControls],
    dependencies: [vulkan_dep, glfw_dep, glslang_dep],
    install: true, install_dir: meson.project_source_root())

benchmarker = executable('benchmarker', ['benchmark.cpp'],
    link_with: [AnACore, AnAControls],
    win_subsystem : 'windows',
    link_args: host_machine.system() == 'windows' and cc.get_id() == 'msvc'
        ? ['/ENTRY:mainCRTStartup']
        : [],
    dependencies: [vulkan_dep, glfw_dep, glslang_dep],
    install: true, install_dir: meson.project_source_root())