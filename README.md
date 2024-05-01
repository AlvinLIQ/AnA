# AnA Game Engine

![Logo](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/AnA_Logo.png "AnA")

## Currently working on

* GUI Controls

## To do

* [X] Lighting System
* [X] Multi-Thread Commandbuffer Recording
* [X] Draw Indirectly
* [ ] Mesh Shader
* [ ] XML to GUI

## Build&Run
Clone the repo and submodules

Install glfw, glm, Vulkan SDK(including validation layer), make, clang(you can use other c/cpp compilers if you want)

### for Linux
```shell
make
./AnA
```

### for Windows
* Install [GNU Make](https://gnuwin32.sourceforge.net/packages/make.htm) for windows(Binaries&Dependencies)
* Edit Makefile (you can skip this step if you're using a MinGW ToolChain)

Sometimes glfw, vulkan lib name can be different, make sure it does match yours

```
make
AnA.exe
```

### Screenshots

![Screenshot1](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004750.png "Screenshot")
![Screenshot2](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004836.png "Screenshot")
