cc = clang
cpp = clang++
cflags = 
libs = 
ana = 
shader =
shaderArgs = --target-env=vulkan1.3
rm =
ifeq ($(OS), Windows_NT)
	libs += -LC:/VulkanSDK/1.3.275.0/Lib -LC:/glfw3/lib-vc2022 -lvulkan-1 -lglfw3dll
	cflags = -IC:/VulkanSDK/1.3.275.0/Include -IC:/glfw3/include
	ana = AnA.exe
	shader = Shaders\\xxdi.exe
	rm = del
else
	libs += -lvulkan -lglfw
	cflags = `pkgconf --cflags vulkan glfw3`
	ana = AnA
	shader = Shaders/xxdi
	rm = rm
endif

sources := $(wildcard Src/Core/*.cpp Src/Core/*/*.cpp Src/GUI/Controls/*.cpp Src/Demo/*.cpp)
objects = $(sources:.cpp=.o)
depends = $(sources:.cpp=.d)

shadercodes := $(wildcard Src/Core/ShaderCodes/*.frag Src/Core/ShaderCodes/*.vert Src/Core/ShaderCodes/*.comp)
temp = $(shadercodes:.vert=_vert.spv)
temp2 = $(temp:.comp=_comp.spv)
shaderspv = $(temp2:.frag=_frag.spv)

all: shader $(ana)

vert:

shader: $(shaderspv)
	$(cc) $(cflags) $(libs) -lSPIRV-Tools-shared Src/Core/ShaderCodes/ShaderCodes.c -o $(shader) -std=c2x
	$(shader) Shaders/ $(^F) > Src/Core/Headers/ShaderCodes.hpp
%_frag.spv : %.frag
	glslc $< -o Shaders/$(@F) $(shaderArgs)
%_vert.spv : %.vert
	glslc $< -o Shaders/$(@F) $(shaderArgs)
%_comp.spv : %.comp
	glslc $< -o Shaders/$(@F) $(shaderArgs)

$(ana): $(objects)
	$(cpp) $^ $(libs) -g -o $@ -std=c++20

-include $(depends)

%.o : %.cpp Makefile
	$(cpp) $(cflags) -g -MMD -MP -c $< -o $@ -std=c++20

clean:
	$(rm) $(objects) $(depends)