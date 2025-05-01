# AnA Game Engine

![Logo](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/AnA_Logo.png "AnA")

## Currently working on

* Culling & LODs

## To do
* [ ] MOVE CASCADED SHADOW MAPPING TO MESH SHADER
* [ ] move descriptor sets model(vertices/indices) to resource manager
* [ ] only pass pointers and count to shader/scene
* [ ] consider using shared pointer for those resources
* [ ] try pipeline cache

## Build&Run
Clone the repo and submodules

Install glfw, glm, Vulkan SDK(including validation layer), make, clang(you can use other c/cpp compilers if you want)

### for Linux
```shell
meson setup build
meson install -C build
./AnA
```

### for Windows
* Install [Meson](https://mesonbuild.com/Getting-meson.html) for windows
* Edit meson.build (you can skip this step if you're using a MinGW ToolChain)

Sometimes glfw, vulkan lib name can be different, make sure it does match yours

```
meson setup build
meson install -C build
AnA.exe
```

### Screenshots

![Screenshot1](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004750.png "Screenshot")
![Screenshot2](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004836.png "Screenshot")
