cc = clang
cpp = clang++
cflags = `pkgconf --cflags vulkan glfw3`
libs = 
ana = 
shader =
ifeq ($(OS), Windows_NT)
	libs += -lvulkan-1.dll -llibglfw3
	ana = AnA.exe
	shader = xxdi.exe
else
	libs += -lvulkan -lglfw
	ana = AnA
	shader = xxdi
endif

sources := $(wildcard Src/Core/*.cpp Src/Core/*/*.cpp Src/GUI/Controls/*.cpp Src/Demo/*.cpp)
objects = $(sources:.cpp=.o)

all: shader $(ana)

shader:
	$(cc) Src/Core/ShaderCodes/ShaderCodes.c -o $(shader) -std=c2x
	./compile.sh
$(ana): $(objects)
	$(cpp) $^ $(libs) -g -o $@ -std=c++20
	rm Src/Core/Pipeline.o

%.o : %.cpp
	$(cpp) $(cflags) -g -c $< -o $@ -std=c++20

clean:
	rm $(objects)