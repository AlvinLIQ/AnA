meshoptimizer_lib = static_library('meshoptimizer', ['meshoptimizer/src/allocator.cpp', 'meshoptimizer/src/partition.cpp', 
    'meshoptimizer/src/vcacheoptimizer.cpp', 
    'meshoptimizer/src/clusterizer.cpp', 'meshoptimizer/src/quantization.cpp', 'meshoptimizer/src/vertexcodec.cpp', 
    'meshoptimizer/src/indexcodec.cpp', 'meshoptimizer/src/simplifier.cpp', 'meshoptimizer/src/vertexfilter.cpp',
    'meshoptimizer/src/indexgenerator.cpp', 'meshoptimizer/src/spatialorder.cpp', 'meshoptimizer/src/vfetchanalyzer.cpp',
    'meshoptimizer/src/overdrawanalyzer.cpp', 'meshoptimizer/src/stripifier.cpp', 'meshoptimizer/src/vfetchoptimizer.cpp',
    'meshoptimizer/src/overdrawoptimizer.cpp', 'meshoptimizer/src/vcacheanalyzer.cpp'], 
    cpp_args: ['-std=c++23'])

vma_include = include_directories('VulkanMemoryAllocator/include', is_system: true)