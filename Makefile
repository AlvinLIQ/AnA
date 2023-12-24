cc = clang++
cflags = `pkgconf --cflags vulkan glfw3`
libs = 
ifeq ($(OS), Windows_NT)
	libs += -lvulkan-1.dll -llibglfw3
else
	libs += -lvulkan -lglfw
endif

sources := $(wildcard Src/Core/*.cpp Src/Core/*/*.cpp Src/GUI/Controls/*.cpp Src/Demo/*.cpp)
objects = $(sources:.cpp=.o)

ana = AnA.exe

all: $(ana)

$(ana): $(objects)
	$(cc) $^ $(libs) -o $@ -std=c++20

%.o : %.cpp
	$(cc) $(cflags) -c $< -o $@ -std=c++20