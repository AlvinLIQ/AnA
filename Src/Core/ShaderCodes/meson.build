fs = import('fs')
shader_sources = files(
    'Mesh.mesh', 'Mesh.task', 'Mesh.frag',
    'Text.task', 'Text.mesh', 'Text.frag'
    )
shader_outputs = []
shader_args = ['--target-env', 'vulkan1.4']
foreach shader_source : shader_sources
  output_file = fs.name(shader_source) + '.spv'
  shader_output = custom_target('compile ' + fs.name(shader_source),
    output: output_file,
    command: ['glslangValidator', '-V', shader_source, '-o', '@OUTPUT@', shader_args],
#    depend_files: [shader_source, 'mesh.h', 'bindings.h']
  )
  shader_outputs += shader_output
endforeach
xxdi = executable('xxdi', 'ShaderCodes.c')
shader_codes_header = 'ShaderCodes.hpp'
shader_hpp = custom_target('Generating Header',
  output: shader_codes_header,
  command: [xxdi, join_paths(meson.project_source_root(), 'Src/Core/Headers/ShaderCodes.hpp'), shader_outputs]
)
