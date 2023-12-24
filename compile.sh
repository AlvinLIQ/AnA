glslc Src/Core/ShaderCodes/Basic.frag -o Shaders/frag.spv
glslc Src/Core/ShaderCodes/Basic.vert -o Shaders/vert.spv
glslc Src/Core/ShaderCodes/PointLight.frag -o Shaders/pointLightFrag.spv
glslc Src/Core/ShaderCodes/PointLight.vert -o Shaders/pointLightVert.spv
glslc Src/Core/ShaderCodes/CollisionDetect.comp -o Shaders/compute.spv
glslc Src/Core/ShaderCodes/Line.vert -o Shaders/lineVert.spv
glslc Src/Core/ShaderCodes/Line.frag -o Shaders/lineFrag.spv

echo const `xxd -i Shaders/frag.spv` > frag.h