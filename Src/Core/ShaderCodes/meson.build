fs = import('fs')
shader_sources = files(
    'Basic.vert', 'Basic.frag', 'CascadedShadowMapping.task', 'CascadedShadowMapping.mesh', 'Point.vert', 'Point.frag',
    'Line.vert', 'Line.frag', 'ShadowMapping.vert', 'Shape.vert', 'Shape.frag', 'Light.comp', 'Light.vert', 'Light.frag', 
    'Mesh.mesh', 'Mesh.task', 'Mesh.frag', 'Terrain.mesh', 'Terrain.task',
    'Text.task', 'Text.mesh', 'Text.frag', 'CollisionDetect.comp')
shader_outputs = []
shader_args = ['--target-env', 'vulkan1.3']
foreach shader_source : shader_sources
  output_file = fs.name(shader_source) + '.spv'
  shader_output = custom_target('compile ' + fs.name(shader_source),
    output: output_file,
    command: ['glslangValidator', '-V', shader_source, '-o', '@OUTPUT@', shader_args],
    depend_files: [shader_source, 'mesh.h', 'math.h']
  )
  shader_outputs += shader_output
endforeach
xxdi = executable('xxdi', 'ShaderCodes.c')
shader_codes_header = 'ShaderCodes.hpp'
shader_hpp = custom_target('Generating Header',
  output: shader_codes_header,
  command: [xxdi, join_paths(meson.project_source_root(), 'Src/Core/Headers/ShaderCodes.hpp'), shader_outputs]
)
