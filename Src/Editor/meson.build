editor_ui = custom_target('Convert UI to cpp code',
  output: 'EditorApp.g.cpp',
  command: [ui_parser, join_paths(meson.project_source_root(), 'Src/Editor/EditorApp.ui'), '@OUTPUT@'],
  depends: [ui_parser], depend_files: ['EditorApp.ui']
  )
AnA = executable('AnA', [editor_ui, 'EditorApp.cpp'],
    link_with: [AnACore, AnAControls],
    dependencies: [vulkan_dep, sdl3_dep],
    cpp_args: cflags,
    include_directories: include_directories('../'),
    install: true, install_dir: meson.project_source_root())
