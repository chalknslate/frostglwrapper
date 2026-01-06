#ifndef VEC_H_
#define VEC_H_
    #include <stdio.h>
    struct Vec3 {
        float x, y, z;
        float (*getX)(struct Vec3* this);
        float (*getY)(struct Vec3* this);
        float (*getZ)(struct Vec3* this);
    };
    float getX(struct Vec3* this) {
        return this->x;
    }
    float getY(struct Vec3* this) {
        return this->y;
    }
    float getZ(struct Vec3* this) {
        return this->z;
    }

    static struct Vec3 newVec3(float x, float y, float z) {
        return (struct Vec3) {
            .x = x,
            .y = y,
            .z = z,
            .getX = &getX,
            .getY = &getY,
            .getZ = &getZ,
        };
    }
    static const struct {
        struct Vec3 (*new)(float x, float y, float z);
    } Vec3 = {.new = &newVec3};

    struct Vec2 {
        float x, y;
        float (*getX)(struct Vec2* this);
        float (*getY)(struct Vec2* this);
    };

    float getX2(struct Vec2* this) {
        return this->x;
    }
    float getY2(struct Vec2* this) {
        return this->y;
    }

    static struct Vec2 newVec2(float x, float y) {
        return (struct Vec2) {
            .x = x,
            .y = y,
            .getX = &getX2,
            .getY = &getY2,
        };
    }
    static const struct {
        struct Vec2 (*new)(float x, float y);
    } Vec2 = {.new = &newVec2};

#endif