#glslc Src/Core/ShaderCodes/Basic.frag -o Shaders/frag.spv
#glslc Src/Core/ShaderCodes/Basic.vert -o Shaders/vert.spv
#glslc Src/Core/ShaderCodes/PointLight.frag -o Shaders/pointLightFrag.spv
#glslc Src/Core/ShaderCodes/PointLight.vert -o Shaders/pointLightVert.spv
#glslc Src/Core/ShaderCodes/CollisionDetect.comp -o Shaders/compute.spv
#glslc Src/Core/ShaderCodes/Line.vert -o Shaders/lineVert.spv
#glslc Src/Core/ShaderCodes/Line.frag -o Shaders/lineFrag.spv
#
#./xxdi Shaders/frag.spv
#./xxdi Shaders/vert.spv
#./xxdi Shaders/compute.spv
#./xxdi Shaders/lineVert.spv
#./xxdi Shaders/lineFrag.spv
head=$'#pragma once\n\n#include <vector>\n'
echo "$head" > Src/Core/Headers/ShaderCodes.hpp
for filename in Src/Core/ShaderCodes/*; do
    base=${filename##*/}
    ext=${base##*.}
    name=${base%.*}
    if [ $ext = "c" ]; then
        continue
    fi
    glslc $filename -o "Shaders/$base.spv"
    ./xxdi "Shaders/$base.spv" >> Src/Core/Headers/ShaderCodes.hpp
done
