#ifndef LIGHT_H_
#define LIGHT_H_
    #include "Vec.h"
    #include "Vector.h"
    #include "Shader.h"
    #include <stdio.h>
    #include <stdlib.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    struct Light {
        struct Vec3 *pos, *color;
        void (*load)(struct Light* this,struct Program* prog);
    };
    void loadLight(struct Light* this, struct Program* prog) {
        struct Uniform lightLocation = Uniform.new(GL_V3F, prog, "lightPos"); 
        struct Uniform lightColor = Uniform.new(GL_V3F, prog, "lightColor");
        lightLocation.ld(&lightLocation, (void*)this->pos);
        lightColor.ld(&lightColor, (void*)this->color);
    }
    inline static struct Light newLight(struct Vec3* pos, struct Vec3 *color) {
        return (struct Light) {
            .pos = pos,
            .color = color,
            .load = &loadLight,
        };
    }
    static const struct {
        struct Light (*new)(struct Vec3 *pos, struct Vec3 *color);
    } Light = { .new = &newLight };
#endif