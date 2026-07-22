# AnA Game Engine

![Logo](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/AnA_Logo.png "AnA")

## Currently working on

* Culling & LODs
* GUI

## To do
* [x] separate shared mesh buffers from Scene
* [x] create submit info queue and only do single submit call every frame
* [ ] optimize resource management before scaling
  * [ ] resource deletion queue
  * [x] rewrite scene buffers with lazy buffer rebuild
  * [ ] use descriptor heap for textures (until mesa 26.2)
  * [x] use device memory address for buffers
  * [ ] make resource manager a facade, real logic in specialized classes.
    * [x] meshes
    * [x] text
    * [ ] textures
    * [x] cameras
* [ ] SSAO
* [ ] occlusion culling
* [ ] try pipeline cache

## Build&Run
Clone the repo and submodules

Install a cpp compiler (tested gcc, clang, msvc)

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

![Screenshot1](https://github.com/AlvinLIQ/AnA_Docs/blob/main/Img/Screenshots/s1.png "Screenshot")
