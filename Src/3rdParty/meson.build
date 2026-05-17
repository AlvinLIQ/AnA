cmake = import('cmake')
sub_options = cmake.subproject_options()

sub_options.add_cmake_defines({
    'BUILD_SHARED_LIBS': 'OFF',
    'ENABLE_CUSTOM_FEATURE': 'ON',
    'SDL_X11': 'OFF',
    'SDL_X11_XCURSOR': 'OFF',
    'SDL_WAYLAND': 'ON'
})
sub_options.set_install(false)
sdl3_proj = cmake.subproject('SDL', options: sub_options)
sdl3_dep = sdl3_proj.dependency('SDL3-static')
sdl3_include = include_directories('SubProjects/SDL/include', is_system: true)

meshoptimizer_lib = static_library('meshoptimizer', ['meshoptimizer/src/allocator.cpp', 'meshoptimizer/src/partition.cpp',
    'meshoptimizer/src/vcacheoptimizer.cpp',
    'meshoptimizer/src/clusterizer.cpp', 'meshoptimizer/src/quantization.cpp', 'meshoptimizer/src/vertexcodec.cpp',
    'meshoptimizer/src/indexcodec.cpp', 'meshoptimizer/src/simplifier.cpp', 'meshoptimizer/src/vertexfilter.cpp',
    'meshoptimizer/src/indexgenerator.cpp', 'meshoptimizer/src/spatialorder.cpp', 'meshoptimizer/src/vfetchanalyzer.cpp',
    'meshoptimizer/src/overdrawanalyzer.cpp', 'meshoptimizer/src/stripifier.cpp', 'meshoptimizer/src/vfetchoptimizer.cpp',
    'meshoptimizer/src/overdrawoptimizer.cpp', 'meshoptimizer/src/vcacheanalyzer.cpp'])

vma_include = include_directories('VulkanMemoryAllocator/include', is_system: true)
cdt_include = include_directories('CDT/CDT/include', is_system: true)
