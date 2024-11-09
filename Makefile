cc = clang
cpp = clang++
cflags = 
libs = 
ana = 
ui_parser =
shader =
shader_args = --target-env=vulkan1.3
rm =
ifeq ($(OS), Windows_NT)
	vulkan_dir = $(wildcard C:/VulkanSDK/*/)
	glfw_dir = C:/glfw3/
	libs += -L$(vulkan_dir)Lib -L$(glfw_dir)lib -lvulkan-1 -lglfw3dll
	cflags = -I$(vulkan_dir)Include -I$(glfw_dir)include
	ana = AnA.exe
	ui_parser = "Src/GUI/XML/ui_parser.exe"
	shader = "Shaders/xxdi.exe"
	rm = del
else
	libs += -lvulkan -lglfw
	cflags = `pkgconf --cflags vulkan glfw3`
	ana = AnA
	shader = Shaders/xxdi
	ui_parser = Src/GUI/XML/ui_parser
	rm = rm
endif

sources := $(wildcard Src/Core/*.cpp Src/Core/*/*.cpp Src/GUI/Controls/*.cpp)
editor := $(wildcard Src/Editors/*.cpp)
ui := $(wildcard Src/Editors/*.ui)
editor += $(ui:.ui=.g.cpp)
example_batching = Src/Examples/example_batching.cpp
objects = $(sources:.cpp=.o)
depends = $(sources:.cpp=.d)

shadercodes := $(wildcard Src/Core/ShaderCodes/*.frag Src/Core/ShaderCodes/*.vert Src/Core/ShaderCodes/*.comp)
temp = $(shadercodes:.vert=_vert.spv)
temp2 = $(temp:.comp=_comp.spv)
shaderspv = $(temp2:.frag=_frag.spv)

all: shader ui $(ana)

shader: shader_prepare $(shader) shader_compile

shader_prepare:
	@ mkdir -p Shaders

shader_compile: $(shaderspv)
	$(shader) Shaders/ $(^F) > Src/Core/Headers/ShaderCodes.hpp

%_frag.spv : %.frag
	glslc $< -o Shaders/$(@F) $(shader_args)
%_vert.spv : %.vert
	glslc $< -o Shaders/$(@F) $(shader_args)
%_comp.spv : %.comp
	glslc $< -o Shaders/$(@F) $(shader_args)

ui: $(ui_parser)
	$(ui_parser) $(ui)

$(shader):
	$(cc) $(cflags) $(libs) -lSPIRV-Tools-shared Src/Core/ShaderCodes/ShaderCodes.c -o $@ -std=c2x

$(ui_parser):
	$(cpp) Src/GUI/XML/XMLToControl.cpp -ISrc/GUI/XML/rapidxml -std=c++20 -o $@

$(ana): $(objects) $(editor:.cpp=.o)
	$(cpp) $^ $(libs) -g -o $@ -std=c++20

example: $(objects) $(example_batching:.cpp=.o)
	$(cpp) $^ $(libs) -g -o $@ -std=c++20

-include $(depends) $(editor:.cpp=.d) $(example_batching:.cpp=.d)

%.o : %.cpp Makefile
	$(cpp) $(cflags) -g -MMD -MP -c $< -o $@ -std=c++20

clean:
	$(rm) $(objects) $(depends) $(editor:.cpp=.o) $(example_batching:.cpp=.o) $(editor:.cpp=.d) $(example_batching:.cpp=.d)