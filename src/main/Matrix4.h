#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
struct Mat4 {
    float m[16];

    // basics
    void (*identity)(struct Mat4* this);
    void (*copy)(struct Mat4* this, struct Mat4* src);
    void (*print)(struct Mat4* this);

    // element access
    void (*set)(struct Mat4* this, int x, int y, float value);
    float (*get)(struct Mat4* this, int x, int y);

    // transforms
    void (*translate)(struct Mat4* this, float x, float y, float z);
    void (*scale)(struct Mat4* this, float sx, float sy, float sz);
    void (*rotate)(struct Mat4* this, float x, float y, float z, float angle);
    void (*multiply)(struct Mat4* this, struct Mat4* other);

    // projections / views
    void (*projection)(struct Mat4* this, float fov, float aspect, float near, float far);
    void (*ortho)(struct Mat4* this, float left, float right, float bottom, float top, float near, float far);
    void (*view)(struct Mat4* this, float x, float y, float z, float rx, float ry, float rz);
    void (*view2D)(struct Mat4* this, float x, float y, float rz);
    void (*transform)(struct Mat4* this, float x, float y, float z,
                      float rx, float ry, float rz,
                      float sx, float sy, float sz);
    void (*transform2D)(struct Mat4* this, float x, float y, float rz, float sx, float sy);
};
static void mat4_identity(struct Mat4* this);
static void mat4_copy(struct Mat4* this, struct Mat4* src);
static void mat4_print(struct Mat4* this);

static void mat4_set(struct Mat4* this, int x, int y, float value);
static float mat4_get(struct Mat4* this, int x, int y);

static void mat4_multiply(struct Mat4* this, struct Mat4* other);

static void mat4_translate(struct Mat4* this, float x, float y, float z);

static void mat4_scale(struct Mat4* this, float sx, float sy, float sz);

static void mat4_rotate(struct Mat4* this, float x, float y, float z, float angle);
// ---- projections / views ----
static void mat4_projection(struct Mat4* this, float fov, float aspect, float near, float far);
static void mat4_ortho(struct Mat4* this, float left, float right, float bottom, float top, float near, float far);
static void mat4_view(struct Mat4* this, float x, float y, float z, float rx, float ry, float rz);

static void mat4_view2D(struct Mat4* this, float x, float y, float rz);

static void mat4_transform(struct Mat4* this, float x, float y, float z,
                           float rx, float ry, float rz,
                           float sx, float sy, float sz);

static void mat4_transform2D(struct Mat4* this, float x, float y, float rz,
                             float sx, float sy);

static struct Mat4 newMat4() {
    struct Mat4 m = {
        .m = {0},
        .identity = &mat4_identity,
        .copy = &mat4_copy,
        .print = &mat4_print,
        .set = &mat4_set,
        .get = &mat4_get,
        .translate = &mat4_translate,
        .scale = &mat4_scale,
        .rotate = &mat4_rotate,
        .multiply = &mat4_multiply,
        .projection = &mat4_projection,
        .ortho = &mat4_ortho,
        .view = &mat4_view,
        .view2D = &mat4_view2D,
        .transform = &mat4_transform,
        .transform2D = &mat4_transform2D
    };
    m.identity(&m);
    return m;
}

static const struct {
    struct Mat4 (*new)();
} Mat4 = {
    .new = &newMat4
};

// ---- helpers ----
static void mat4_identity(struct Mat4* this) {
    float id[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    memcpy(this->m, id, sizeof(id));
}

static void mat4_copy(struct Mat4* this, struct Mat4* src) {
    memcpy(this->m, src->m, sizeof(float)*16);
}

static void mat4_print(struct Mat4* this) {
    for (int row = 0; row < 4; row++) {
        printf("| %6.2f %6.2f %6.2f %6.2f |\n",
            this->m[row*4+0], this->m[row*4+1],
            this->m[row*4+2], this->m[row*4+3]);
    }
    printf("\n");
}

static void mat4_set(struct Mat4* this, int x, int y, float value) {
    this->m[y*4+x] = value;
}
static float mat4_get(struct Mat4* this, int x, int y) {
    return this->m[y*4+x];
}

static void mat4_multiply(struct Mat4* this, struct Mat4* other) {
    float temp[16] = {0};
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            for (int k = 0; k < 4; k++) {
                temp[row*4+col] += this->m[row*4+k] * other->m[k*4+col];
            }
        }
    }
    memcpy(this->m, temp, sizeof(temp));
}

static void mat4_translate(struct Mat4* this, float x, float y, float z) {
    struct Mat4 t = Mat4.new();
    t.set(&t, 3,0,x);
    t.set(&t, 3,1,y);
    t.set(&t, 3,2,z);
    this->multiply(this, &t);
}

static void mat4_scale(struct Mat4* this, float sx, float sy, float sz) {
    struct Mat4 s = Mat4.new();
    s.set(&s, 0,0,sx);
    s.set(&s, 1,1,sy);
    s.set(&s, 2,2,sz);
    this->multiply(this, &s);
}

static void mat4_rotate(struct Mat4* this, float x, float y, float z, float angle) {
    float rad = angle * (float)M_PI / 180.0f;
    float co = cosf(rad);
    float si = sinf(rad);
    float C  = 1.0f - co;

    struct Mat4 r = Mat4.new();
    r.set(&r, 0,0, co + x*x*C);
    r.set(&r, 0,1, x*y*C - z*si);
    r.set(&r, 0,2, x*z*C + y*si);

    r.set(&r, 1,0, y*x*C + z*si);
    r.set(&r, 1,1, co + y*y*C);
    r.set(&r, 1,2, y*z*C - x*si);

    r.set(&r, 2,0, z*x*C - y*si);
    r.set(&r, 2,1, z*y*C + x*si);
    r.set(&r, 2,2, co + z*z*C);

    this->multiply(this, &r);
}

// ---- projections / views ----
static void mat4_projection(struct Mat4* this, float fov, float aspect, float near, float far) {
    this->identity(this);
    float t = tanf((fov * (float)M_PI / 180.0f) / 2.0f);
    float range = far - near;
    this->set(this,0,0, 1.0f / (aspect * t));
    this->set(this,1,1, 1.0f / t);
    this->set(this,2,2, -(far + near) / range);
    this->set(this,2,3, -1.0f);
    this->set(this,3,2, -(2*far*near) / range);
    this->set(this,3,3, 0.0f);
}

static void mat4_ortho(struct Mat4* this, float left, float right, float bottom, float top, float near, float far) {
    this->identity(this);
    this->set(this,0,0, 2.0f / (right-left));
    this->set(this,1,1, 2.0f / (top-bottom));
    this->set(this,2,2, -2.0f / (far-near));
    this->set(this,3,0, -(right+left)/(right-left));
    this->set(this,3,1, -(top+bottom)/(top-bottom));
    this->set(this,3,2, -(far+near)/(far-near));
}

static void mat4_view(struct Mat4* this, float x, float y, float z, float rx, float ry, float rz) {
    this->identity(this);
    this->rotate(this, 1,0,0, rx);
    this->rotate(this, 0,1,0, ry);
    this->rotate(this, 0,0,1, rz);
    this->translate(this, -x, -y, -z);
}

static void mat4_view2D(struct Mat4* this, float x, float y, float rz) {
    this->identity(this);
    this->translate(this, -x, -y, 0);
    this->rotate(this, 0,0,1, rz);
}

static void mat4_transform(struct Mat4* this, float x, float y, float z,
                           float rx, float ry, float rz,
                           float sx, float sy, float sz) {
    this->identity(this);
    this->translate(this, x, y, z);
    this->rotate(this, 1,0,0, rx);
    this->rotate(this, 0,1,0, ry);
    this->rotate(this, 0,0,1, rz);
    this->scale(this, sx, sy, sz);
}

static void mat4_transform2D(struct Mat4* this, float x, float y, float rz,
                             float sx, float sy) {
    this->identity(this);
    this->translate(this, x, y, 0);
    this->rotate(this, 0,0,1, rz);
    this->scale(this, sx, sy, 1);
}

// ---- constructor ----

#endif
