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
