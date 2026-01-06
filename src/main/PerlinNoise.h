#ifndef PERLINNOISE_H_
#define PERLINNOISE_H_

#include "Vec.h"
#include "Vector.h"
#include <math.h>
#include <stdlib.h>
static inline float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static inline float gradDot(struct Vec2 g, float x, float y) {
    return g.x * x + g.y * y;
}

struct PerlinNoise {
    int dimension;
    struct Vector vectors;
    float (*noiseAt)(struct PerlinNoise* this, struct Vec2 position);
    void (*destroy)(struct PerlinNoise* this);
};

static float perlinNoiseAt(struct PerlinNoise* this, struct Vec2 position) {
    int dim = this->dimension;

    int xi = (int)floorf(position.x) % dim;
    int yi = (int)floorf(position.y) % dim;
    if (xi < 0) xi += dim;
    if (yi < 0) yi += dim;

    float xf = position.x - floorf(position.x);
    float yf = position.y - floorf(position.y);

    float u = fade(xf);
    float v = fade(yf);

    struct Vec2 g00 = ((struct Vec2*)this->vectors.data)[xi + yi * dim];
    struct Vec2 g10 = ((struct Vec2*)this->vectors.data)[(xi + 1) % dim + yi * dim];
    struct Vec2 g01 = ((struct Vec2*)this->vectors.data)[xi + ((yi + 1) % dim) * dim];
    struct Vec2 g11 = ((struct Vec2*)this->vectors.data)[(xi + 1) % dim + ((yi + 1) % dim) * dim];

    float n00 = gradDot(g00, xf, yf);
    float n10 = gradDot(g10, xf - 1, yf);
    float n01 = gradDot(g01, xf, yf - 1);
    float n11 = gradDot(g11, xf - 1, yf - 1);

    float x1 = lerp(n00, n10, u);
    float x2 = lerp(n01, n11, u);
    return lerp(x1, x2, v);
}

static void destroyPerlin(struct PerlinNoise* this) {
    this->vectors.destroy(&this->vectors);
}

inline static struct PerlinNoise newPerlinNoise(int dimension) {
    struct PerlinNoise pn;
    pn.dimension = dimension;
    pn.vectors = Vector.new(dimension * dimension, FIELD_TYPE_VEC2);

    for (int y = 0; y < dimension; y++) {
        for (int x = 0; x < dimension; x++) {
            float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            struct Vec2 g = Vec2.new(cosf(angle), sinf(angle));
            pn.vectors.push_back(&pn.vectors, &g);
        }
    }

    pn.noiseAt = &perlinNoiseAt;
    pn.destroy = &destroyPerlin;
    return pn;
}

static const struct {
    struct PerlinNoise (*new)(int dimension);
} PerlinNoise = {
    .new = &newPerlinNoise
};

#endif
