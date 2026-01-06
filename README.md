# frostglwrapper
A OpenGL wrapper written originally for C. Header-only implementation wise, but you will need glfw and glad. I did not write it to use different loaders. Handles 3D scenes, ease of model and window initiation, and a built-in OBJ loader with small amounts of window customization.

Features:

Class-like model initialization using the ModelDataInfo struct.
An example of loading a model:
```
//example.c
#include <stdio.h>
#include "LoadedModel.h"
int main(int argc, char*argv[])
{
    initialize();
    struct Vec3 position = Vec3.new(0,0,0);
    struct Vec3 rotation = Vec3.new(0,0,0);
    
    /* create a model from an OBJ file */ struct LoadedModel sliceOfBread = LoadedModel.new("sliceOfBread.obj",&position,&rotation);
    fr_exit();
    return 0;
}
```
Handles uniforms and shaders wrapped into structs.