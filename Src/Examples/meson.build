benchmarker = executable('benchmarker', ['benchmark.cpp'],
    link_with: [AnACore, AnAControls],
    win_subsystem : 'windows',
    link_args: host_machine.system() == 'windows' and cc.get_id() == 'msvc'
        ? ['/ENTRY:mainCRTStartup']
        : [],
    dependencies: [vulkan_dep, sdl3_dep],
    install: true, install_dir: meson.project_source_root())
