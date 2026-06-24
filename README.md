# AnA Game Engine

![Logo](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/AnA_Logo.png "AnA")

## Currently working on

* Culling & LODs
* GUI

## To do
* [x] separate shared mesh buffers from Scene
* [x] create submit info queue and only do single submit call every frame
* [ ] optimize resource management before scaling
  * [x] rewrite scene buffers with lazy buffer rebuild
  * [ ] make resource manager a facade, real logic in specialized classes.
    * [x] meshes
    * [x] text
    * [ ] textures
    * [x] cameras
    * [ ] lights
* [ ] use ray tracing instead of traditional shadow map when having multiple light sources (enable by default when mesh shader is supported)
* [ ] SSAO
* [ ] occlusion culling
* [ ] try pipeline cache

## Build&Run
Clone the repo and submodules

Install glm, Vulkan SDK(including validation layer), clang(you can use other c/cpp compilers if you want)

### for Linux
```shell
meson setup build
meson install -C build
./AnA
```

### for Windows
* Install [Meson](https://mesonbuild.com/Getting-meson.html) for windows

Sometimes vulkan lib name can be different, make sure it does match yours

```
meson setup build
meson install -C build
AnA.exe
```

### Screenshots

![Screenshot1](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004750.png "Screenshot")
![Screenshot2](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/Screenshot_20240416_004836.png "Screenshot")
