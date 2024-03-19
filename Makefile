cc = clang
cpp = clang++
cflags = 
libs = 
ana = 
shader =
echoheader = 
ifeq ($(OS), Windows_NT)
	libs += -LC:/VulkanSDK/1.3.275.0/Lib -LC:/glfw3/lib-vc2022 -lvulkan-1 -lglfw3dll
	cflags = -IC:/VulkanSDK/1.3.275.0/Include -IC:/glfw3/include
	ana = AnA.exe
	shader = xxdi.exe
	echoheader = echo \#include ^<vector^>
else
	libs += -lvulkan -lglfw
	cflags = `pkgconf --cflags vulkan glfw3`
	ana = AnA
	shader = xxdi
	echoheader = echo "\#include <vector>"
endif

sources := $(wildcard Src/Core/*.cpp Src/Core/*/*.cpp Src/GUI/Controls/*.cpp Src/Demo/*.cpp)
objects = $(sources:.cpp=.o)

shadercodes := $(wildcard Src/Core/ShaderCodes/*.frag Src/Core/ShaderCodes/*.vert Src/Core/ShaderCodes/*.comp)
temp = $(shadercodes:.vert=_vert.spv)
temp2 = $(temp:.comp=_comp.spv)
shaderspv = $(temp2:.frag=_frag.spv)

all: shader $(ana)

vert:

shader: $(shaderspv)
	$(cc) Src/Core/ShaderCodes/ShaderCodes.c -o $(shader) -std=c2x
	xxdi Shaders/ $(^F) > Src/Core/Headers/ShaderCodes.hpp
%_frag.spv : %.frag
	glslc $< -o Shaders/$(@F)
%_vert.spv : %.vert
	glslc $< -o Shaders/$(@F)
%_comp.spv : %.comp
	glslc $< -o Shaders/$(@F)

$(ana): $(objects)
	$(cpp) $^ $(libs) -g -o $@ -std=c++20

%.o : %.cpp
	$(cpp) $(cflags) -g -c $< -o $@ -std=c++20

clean:
	rm $(objects)