#ifndef CHUNK_H_
#define CHUNK_H_

#include "Model.h"
#include "Vec.h"
#include "Vector.h"
#include <stdint.h>

// Face normals
static const float FACE_NORMALS[6][3] = {
    { 0,  0,  1}, // FRONT
    { 0,  0, -1}, // BACK
    {-1,  0,  0}, // LEFT
    { 1,  0,  0}, // RIGHT
    { 0,  1,  0}, // TOP
    { 0, -1,  0}  // BOTTOM
};

// Cube vertices per face
static const float CUBE_FACE_VERTS[6][4][3] = {
    // FRONT
    {{0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}},
    // BACK
    {{1,0,0}, {0,0,0}, {1,1,0}, {0,1,0}},
    // LEFT
    {{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}},
    // RIGHT
    {{1,0,1}, {1,0,0}, {1,1,1}, {1,1,0}},
    // TOP
    {{0,1,1}, {1,1,1}, {0,1,0}, {1,1,0}},
    // BOTTOM
    {{0,0,0}, {1,0,0}, {0,0,1}, {1,0,1}}
};

// UVs per face
static const float CUBE_FACE_UVS[4][2] = {
    {0,0}, {1,0}, {0,1}, {1,1}
};

// Getter helpers
static inline struct Vec3 getFaceNormal(int dir) {
    return Vec3.new(FACE_NORMALS[dir][0], FACE_NORMALS[dir][1], FACE_NORMALS[dir][2]);
}

static inline struct Vec3 getFaceVertex(int dir, int v) {
    return Vec3.new(
        CUBE_FACE_VERTS[dir][v][0],
        CUBE_FACE_VERTS[dir][v][1],
        CUBE_FACE_VERTS[dir][v][2]
    );
}

static inline struct Vec2 getFaceUV(int v) {
    return Vec2.new(CUBE_FACE_UVS[v][0], CUBE_FACE_UVS[v][1]);
}

#define CHUNK_SIZE 8
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef uint8_t Block; // 0 = air, >0 = solid
#define CHUNK_INDEX(x, y, z) ((x) + (y) * CHUNK_SIZE + (z) * CHUNK_SIZE * CHUNK_SIZE)

struct Chunk;

struct Model (*ChunkMeshFunc)(struct Chunk* this);

struct Chunk {
    Block blocks[CHUNK_VOLUME];
    struct Model mesh;
    struct Model (*meshify)(struct Chunk* this);
    struct Vec2* position;
};

// helper — check face visibility
static inline int isFaceVisible(struct Chunk* chunk, int x, int y, int z, int dir) {
    int nx = x + (int)FACE_NORMALS[dir][0];
    int ny = y + (int)FACE_NORMALS[dir][1];
    int nz = z + (int)FACE_NORMALS[dir][2];

    if (nx < 0 || ny < 0 || nz < 0 ||
        nx >= CHUNK_SIZE || ny >= CHUNK_SIZE || nz >= CHUNK_SIZE)
        return 1; // visible at edge

    int nIndex = CHUNK_INDEX(nx, ny, nz);
    return chunk->blocks[nIndex] == 0;
}

// Vec3 add helper
static inline struct Vec3 Vec3_add(struct Vec3 a, struct Vec3 b) {
    return Vec3.new(a.x + b.x, a.y + b.y, a.z + b.z);
}

// meshing
static struct Model meshifyChunk(struct Chunk* this) {
    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);
    
    int indexOffset = 0;

    const int atlasN = 2; // 2x2 atlas (4 textures)
    const float atlasSize = 1.0f / (float)atlasN;

    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                int i = CHUNK_INDEX(x, y, z);
                int blockType = this->blocks[i];
                if (blockType == 0) continue; // air

                // compute UV offset in the atlas
                int tx = blockType % atlasN;
                int ty = blockType / atlasN;
                float uOffset = tx * atlasSize;
                float vOffset = ty * atlasSize;

                for (int dir = 0; dir < 6; dir++) {
                    if (!isFaceVisible(this, x, y, z, dir))
                        continue;

                    struct Vec3 base = Vec3.new(x, y, z);
                    struct Vec3 normal = getFaceNormal(dir);

                    for (int v = 0; v < 4; v++) {
                        struct Vec3 vert = Vec3_add(getFaceVertex(dir, v), base);
                        vertices.push_back(&vertices, &vert);
                        normals.push_back(&normals, &normal);

                        struct Vec2 uvBase = getFaceUV(v);
                        struct Vec2 uv = Vec2.new(
                            uvBase.x * atlasSize + uOffset,
                            uvBase.y * atlasSize + vOffset
                        );
                        uvs.push_back(&uvs, &uv);
                    }

                    int inds[6] = {0, 1, 2, 2, 1, 3};
                    for (int j = 0; j < 6; j++) {
                        int idx = indexOffset + inds[j];
                        indices.push_back(&indices, &idx);
                    }

                    indexOffset += 4;
                }
            }
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    if (vertices.size == 0) {
        return Model.new(); // empty model
    }

    model.ld(&model, &v, &i, &uv, &n);

    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}


// constructor
inline static struct Chunk newChunk(struct Vec2* position) {
    struct Chunk c = {0};
    c.meshify = &meshifyChunk;
    c.position = position;
    return c;
}

static const struct {
    struct Chunk (*new)(struct Vec2* position);
} Chunk = { .new = &newChunk };
#include "PerlinNoise.h"
#include "Model.h"
#include "Vector.h"
#include "Vec.h"
#include "stb_image.h"

float* Chunk_loadHeightMap(const char* path, int* width, int* height) {
    int n;
    unsigned char* data = stbi_load(path, width, height, &n, 1); // force grayscale
    if (!data) {
        printf("Failed to load heightmap: %s\n", path);
        return NULL;
    }

    float* heights = malloc((*width) * (*height) * sizeof(float));
    for (int i = 0; i < (*width) * (*height); i++) {
        heights[i] = data[i] / 255.0f; // normalize 0–1
    }

    stbi_image_free(data);
    return heights;
}

struct Model Chunk_generateFlatPlane(void) {
    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const int TILE_COUNT_X = 256;
    const int TILE_COUNT_Z = TILE_COUNT_X;
    const float TILE_SIZE = 1.0f;
    const float TILE_SCALE = 1.0f;  // how often texture repeats

    int indexOffset = 0;

    for (int z = 0; z < TILE_COUNT_Z; z++) {
        for (int x = 0; x < TILE_COUNT_X; x++) {
            float x0 = x * TILE_SIZE;
            float x1 = x0 + TILE_SIZE;
            float z0 = z * TILE_SIZE;
            float z1 = z0 + TILE_SIZE;
            float y  = 0.0f;

            struct Vec3 normal = Vec3.new(0, 1, 0);

            struct Vec3 v0 = Vec3.new(x0, y, z0);
            struct Vec3 v1 = Vec3.new(x1, y, z0);
            struct Vec3 v2 = Vec3.new(x0, y, z1);
            struct Vec3 v3 = Vec3.new(x1, y, z1);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // Tile UVs so textures repeat nicely
            // Use normalized UVs that stretch across the entire terrain
            struct Vec2 uv0 = Vec2.new((float)x / TILE_COUNT_X, (float)z / TILE_COUNT_Z);
            struct Vec2 uv1 = Vec2.new((float)(x + 1) / TILE_COUNT_X, (float)z / TILE_COUNT_Z);
            struct Vec2 uv2 = Vec2.new((float)x / TILE_COUNT_X, (float)(z + 1) / TILE_COUNT_Z);
            struct Vec2 uv3 = Vec2.new((float)(x + 1) / TILE_COUNT_X, (float)(z + 1) / TILE_COUNT_Z);

            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                int idx = indexOffset + inds[i];
                indices.push_back(&indices, &idx);
            }

            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}
struct Model Chunk_generateTexturePlaneSEE(const char* idMapPath) {
    int width = 128;
    int height = 128;

    // Load texture IDs from file
    uint8_t textureIDs[16384]; // 128*128
    FILE* f = fopen(idMapPath, "r");
    if (!f) {
        printf("Failed to open file: %s\n", idMapPath);
    } else {
        fread(textureIDs, 1, 16384, f);
        fclose(f);
    }

    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const float TILE_SIZE = 1.0f;
    const float HEIGHT = 0.0f; // flat plane

    const int atlasN = 2;             // atlas is 2x2 tiles
    const float tileSize = 1.0f / atlasN;

    int indexOffset = 0;

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {

            // get terrain ID
            int id = textureIDs[z * width + x];

            // positions
            struct Vec3 v0 = Vec3.new(x * TILE_SIZE, HEIGHT, z * TILE_SIZE);
            struct Vec3 v1 = Vec3.new((x + 1) * TILE_SIZE, HEIGHT, z * TILE_SIZE);
            struct Vec3 v2 = Vec3.new(x * TILE_SIZE, HEIGHT, (z + 1) * TILE_SIZE);
            struct Vec3 v3 = Vec3.new((x + 1) * TILE_SIZE, HEIGHT, (z + 1) * TILE_SIZE);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            // normals
            struct Vec3 normal = Vec3.new(0, 1, 0);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // atlas UVs based on ID
            int tx = id % atlasN;
            int ty = id / atlasN;

            struct Vec2 uv0 = Vec2.new(0.0f * tileSize + tx * tileSize,
                                       0.0f * tileSize + ty * tileSize);
            struct Vec2 uv1 = Vec2.new(1.0f * tileSize + tx * tileSize,
                                       0.0f * tileSize + ty * tileSize);
            struct Vec2 uv2 = Vec2.new(0.0f * tileSize + tx * tileSize,
                                       1.0f * tileSize + ty * tileSize);
            struct Vec2 uv3 = Vec2.new(1.0f * tileSize + tx * tileSize,
                                       1.0f * tileSize + ty * tileSize);

            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            // indices
            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                int idx = indexOffset + inds[i];
                indices.push_back(&indices, &idx);
            }
            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}

struct Model Chunk_generateHeightmapPlaneSE( const char* heightmapPath,int* terrainIDs) {
    int width, height;
    uint8_t heightData[16384];
    FILE* f = fopen(heightmapPath, "r");
    if (!f) {
        printf("Failed to open output file: %s\n", heightmapPath);
    }
    int idMapSize = 32;
    fread(heightData, 1, 16384, f);
    fclose(f);
    // float* heightData = Chunk_loadHeightMap(heightmapPath, &width, &height);
    // if (!heightData) return Model.new();
    width = 128;
     height = 128;


    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const float TILE_SIZE = 1.0f;
    const float HEIGHT_SCALE = 30.0f;

    const int atlasN = 2;             // atlas is 4×4 tiles
    const float tileSize = 1.0f / atlasN;
    const float uvRepeat = 1.0f;      // 1 = each tile uses full atlas cell

    int indexOffset = 0;

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {

            // pick ID from smaller map based on scaled position
            int tileX = (int)((float)x / (width - 1) * idMapSize);
            int tileZ = (int)((float)z / (height - 1) * idMapSize);

            // clamp to valid range
            if (tileX >= idMapSize) tileX = idMapSize - 1;
            if (tileZ >= idMapSize) tileZ = idMapSize - 1;

            int terrainID = terrainIDs[tileZ * idMapSize + tileX];

            // --- get height from map ---
            float y00 = (heightData[z * width + x]/255.0f) * HEIGHT_SCALE;
            float y10 = (heightData[z * width + (x + 1)]/255.0f)  * HEIGHT_SCALE;
            float y01 = (heightData[(z + 1) * width + x]/255.0f)  * HEIGHT_SCALE;
            float y11 = (heightData[(z + 1) * width + (x + 1)]/255.0f) * HEIGHT_SCALE;

            // --- positions ---
            struct Vec3 v0 = Vec3.new(x * TILE_SIZE, y00, z * TILE_SIZE);
            struct Vec3 v1 = Vec3.new((x + 1) * TILE_SIZE, y10, z * TILE_SIZE);
            struct Vec3 v2 = Vec3.new(x * TILE_SIZE, y01, (z + 1) * TILE_SIZE);
            struct Vec3 v3 = Vec3.new((x + 1) * TILE_SIZE, y11, (z + 1) * TILE_SIZE);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            // --- normals (flat for now) ---
            struct Vec3 normal = Vec3.new(0, 1, 0);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // --- atlas UVs based on terrainID ---
            int tx = terrainID % atlasN;
            int ty = terrainID / atlasN;

            struct Vec2 uv0 = Vec2.new(0.0f * tileSize + tx * tileSize,
                                       0.0f * tileSize + ty * tileSize);
            struct Vec2 uv1 = Vec2.new(1.0f * tileSize + tx * tileSize,
                                       0.0f * tileSize + ty * tileSize);
            struct Vec2 uv2 = Vec2.new(0.0f * tileSize + tx * tileSize,
                                       1.0f * tileSize + ty * tileSize);
            struct Vec2 uv3 = Vec2.new(1.0f * tileSize + tx * tileSize,
                                       1.0f * tileSize + ty * tileSize);

            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            // --- indices ---
            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                int idx = indexOffset + inds[i];
                indices.push_back(&indices, &idx);
            }
            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    //free(heightData);
    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}


struct Model Chunk_generateHeightmapPlaneS(const char* heightmapPath, int* terrainIDs, int width, int height) {
    float* heightData = Chunk_loadHeightMap(heightmapPath, &width, &height);
    if (!heightData) return Model.new();

    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const float TILE_SIZE = 1.0f;
    const float HEIGHT_SCALE = 45.0f; // mountains height
    const int atlasN = 2;             // e.g., 4x4 atlas
    const float tileSize = 1.0f / atlasN;
    const float uvRepeat = 1.0f;      // repeats per tile

    int indexOffset = 0;

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            // heights
            float y00 = heightData[z * width + x] * HEIGHT_SCALE;
            float y10 = heightData[z * width + (x + 1)] * HEIGHT_SCALE;
            float y01 = heightData[(z + 1) * width + x] * HEIGHT_SCALE;
            float y11 = heightData[(z + 1) * width + (x + 1)] * HEIGHT_SCALE;

            // vertices
            struct Vec3 v0 = Vec3.new(x * TILE_SIZE, y00, z * TILE_SIZE);
            struct Vec3 v1 = Vec3.new((x + 1) * TILE_SIZE, y10, z * TILE_SIZE);
            struct Vec3 v2 = Vec3.new(x * TILE_SIZE, y01, (z + 1) * TILE_SIZE);
            struct Vec3 v3 = Vec3.new((x + 1) * TILE_SIZE, y11, (z + 1) * TILE_SIZE);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            // flat normals
            struct Vec3 normal = Vec3.new(0, 1, 0);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // --- texture UVs based on terrainID ---
            const float tileSize = 1.0f / atlasN;

            // fixed UVs per terrain ID
            struct Vec2 uvTiles[16][4] = {
                // terrainID 0
                { Vec2.new(0, 0), Vec2.new(tileSize, 0), Vec2.new(0, tileSize), Vec2.new(tileSize, tileSize) },
                // terrainID 1
                { Vec2.new(tileSize, 0), Vec2.new(2*tileSize, 0), Vec2.new(tileSize, tileSize), Vec2.new(2*tileSize, tileSize) },
                // terrainID 2
                { Vec2.new(0, tileSize), Vec2.new(tileSize, tileSize), Vec2.new(0, 2*tileSize), Vec2.new(tileSize, 2*tileSize) },
                // terrainID 3 ...
                // fill out the rest as needed
            };

            int terrainID = terrainIDs[z * width + x]; // quad's ID

            struct Vec2 uv0 = uvTiles[terrainID][0];
            struct Vec2 uv1 = uvTiles[terrainID][1];
            struct Vec2 uv2 = uvTiles[terrainID][2];
            struct Vec2 uv3 = uvTiles[terrainID][3];

            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            // indices
            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                indices.push_back(&indices, &(int){indexOffset + inds[i]});
            }
            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    free(heightData);
    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}




struct Model Chunk_generateHeightmapPlaneN(const char* heightmapPath) {
    int width, height;
    float* heightData = Chunk_loadHeightMap(heightmapPath, &width, &height);
    if (!heightData) return Model.new();

    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const float TILE_SIZE = 1.0f;
    const float HEIGHT_SCALE = 130.0f;
    const int atlasN = 2;          // 4x4 atlas (16 tiles)
    const float tileSize = 1.0f / atlasN;
    const float uvRepeat = 8.0f;   // how many times texture repeats over the terrain

    int indexOffset = 0;

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            float y00 = heightData[z * width + x] * HEIGHT_SCALE;
            float y10 = heightData[z * width + (x + 1)] * HEIGHT_SCALE;
            float y01 = heightData[(z + 1) * width + x] * HEIGHT_SCALE;
            float y11 = heightData[(z + 1) * width + (x + 1)] * HEIGHT_SCALE;

            struct Vec3 v0 = Vec3.new(x * TILE_SIZE, y00, z * TILE_SIZE);
            struct Vec3 v1 = Vec3.new((x + 1) * TILE_SIZE, y10, z * TILE_SIZE);
            struct Vec3 v2 = Vec3.new(x * TILE_SIZE, y01, (z + 1) * TILE_SIZE);
            struct Vec3 v3 = Vec3.new((x + 1) * TILE_SIZE, y11, (z + 1) * TILE_SIZE);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            // flat normals (for now)
            struct Vec3 normal = Vec3.new(0, 1, 0);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // --- Compute terrain type per vertex based on height ---
            int terrain00 = (y00 > 20.0f) ? 0 : 2;
            int terrain10 = (y10 > 20.0f) ? 0 : 2;
            int terrain01 = (y01 > 20.0f) ? 0 : 2;
            int terrain11 = (y11 > 20.0f) ? 0 : 2;

            // --- Map terrain to atlas tiles ---
            int tx0 = terrain00 % atlasN; int ty0 = terrain00 / atlasN;
            int tx1 = terrain10 % atlasN; int ty1 = terrain10 / atlasN;
            int tx2 = terrain01 % atlasN; int ty2 = terrain01 / atlasN;
            int tx3 = terrain11 % atlasN; int ty3 = terrain11 / atlasN;

            // --- Compute UVs inside each tile, scaled for tiling ---
            struct Vec2 uv0 = Vec2.new(fmodf((float)x / uvRepeat, 1.0f) * tileSize + tx0 * tileSize,
                                       fmodf((float)z / uvRepeat, 1.0f) * tileSize + ty0 * tileSize);
            struct Vec2 uv1 = Vec2.new(fmodf((float)(x + 1) / uvRepeat, 1.0f) * tileSize + tx1 * tileSize,
                                       fmodf((float)z / uvRepeat, 1.0f) * tileSize + ty1 * tileSize);
            struct Vec2 uv2 = Vec2.new(fmodf((float)x / uvRepeat, 1.0f) * tileSize + tx2 * tileSize,
                                       fmodf((float)(z + 1) / uvRepeat, 1.0f) * tileSize + ty2 * tileSize);
            struct Vec2 uv3 = Vec2.new(fmodf((float)(x + 1) / uvRepeat, 1.0f) * tileSize + tx3 * tileSize,
                                       fmodf((float)(z + 1) / uvRepeat, 1.0f) * tileSize + ty3 * tileSize);

            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                int v = (indexOffset + inds[i]);
                indices.push_back(&indices, (void*)&v);
            }
            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    free(heightData);
    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}

struct Model Chunk_generateHeightmapPlane(const char* heightmapPath) {
    int width, height;
    float* heightData = Chunk_loadHeightMap(heightmapPath, &width, &height);
    if (!heightData) return Model.new();

    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);

    const float TILE_SIZE = 2.0f;
    const float HEIGHT_SCALE = 30.0f; // adjust for how tall mountains are

    int indexOffset = 0;

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            float y00 = heightData[z * width + x] * HEIGHT_SCALE;
            float y10 = heightData[z * width + (x + 1)] * HEIGHT_SCALE;
            float y01 = heightData[(z + 1) * width + x] * HEIGHT_SCALE;
            float y11 = heightData[(z + 1) * width + (x + 1)] * HEIGHT_SCALE;

            struct Vec3 v0 = Vec3.new(x * TILE_SIZE, y00, z * TILE_SIZE);
            struct Vec3 v1 = Vec3.new((x + 1) * TILE_SIZE, y10, z * TILE_SIZE);
            struct Vec3 v2 = Vec3.new(x * TILE_SIZE, y01, (z + 1) * TILE_SIZE);
            struct Vec3 v3 = Vec3.new((x + 1) * TILE_SIZE, y11, (z + 1) * TILE_SIZE);

            vertices.push_back(&vertices, &v0);
            vertices.push_back(&vertices, &v1);
            vertices.push_back(&vertices, &v2);
            vertices.push_back(&vertices, &v3);

            // simple flat normals (you can later compute smooth ones)
            struct Vec3 normal = Vec3.new(0, 1, 0);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);
            normals.push_back(&normals, &normal);

            // UV coordinates normalized across terrain
            // --- Atlas parameters ---
            const int atlasN = 64;                     // 4x4 tiles
            const float tileSize = 1.0f / atlasN;     // each tile covers 0.25 UV units

            // choose the atlas tile for this terrain (example: tile at grid (1, 2))
            const int tileX = 1;
            const int tileY = 1;

            // compute offsets into atlas
            const float uOffset = tileX * tileSize;
            const float vOffset = tileY * tileSize;

            // repeat UVs across the tile (you can tweak scale if desired)
            float uvScale = 8.0f;  // controls how many times texture repeats across the terrain
            float u = ((float)x / uvScale) * tileSize + uOffset;
            float v = ((float)z / uvScale) * tileSize + vOffset;

            struct Vec2 uv0 = Vec2.new(u, v);
            struct Vec2 uv1 = Vec2.new(u + tileSize / uvScale, v);
            struct Vec2 uv2 = Vec2.new(u, v + tileSize / uvScale);
            struct Vec2 uv3 = Vec2.new(u + tileSize / uvScale, v + tileSize / uvScale);



            uvs.push_back(&uvs, &uv0);
            uvs.push_back(&uvs, &uv1);
            uvs.push_back(&uvs, &uv2);
            uvs.push_back(&uvs, &uv3);

            int inds[6] = {0, 1, 2, 2, 1, 3};
            for (int i = 0; i < 6; i++) {
                int idx = indexOffset + inds[i];
                indices.push_back(&indices, &idx);
            }
            indexOffset += 4;
        }
    }

    struct ModelDataInfo v  = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo uv = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);
    struct ModelDataInfo n  = ModelDataInfo.new(normals.data, ENG_VEC3, normals.size);
    struct ModelDataInfo i  = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

    struct Model model = Model.new();
    model.ld(&model, &v, &i, &uv, &n);

    free(heightData);
    vertices.destroy(&vertices);
    normals.destroy(&normals);
    uvs.destroy(&uvs);
    indices.destroy(&indices);

    return model;
}

void Chunk_generateTerrain(struct Chunk* this) {
    // scale height and noise frequency based on chunk size
    const int MAX_HEIGHT = CHUNK_SIZE / 2;        // top half of chunk used for terrain
    const float FREQUENCY = 0.05f * (64.0f / CHUNK_SIZE); // keep noise consistent across sizes
    const float AMPLITUDE = (float)(MAX_HEIGHT - 2);

    struct PerlinNoise noise = PerlinNoise.new(CHUNK_SIZE);

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            struct Vec2 samplePos = Vec2.new(x * FREQUENCY, z * FREQUENCY);
            float n = noise.noiseAt(&noise, samplePos);
            n = (n + 1.0f) * 0.5f; // normalize to 0–1
            int height = (int)(n * AMPLITUDE);

            for (int y = 0; y < CHUNK_SIZE; y++) {
                if (y < height - 1)
                    this->blocks[CHUNK_INDEX(x, y, z)] = 4; // deeper solid
                else if (y == height - 1)
                    this->blocks[CHUNK_INDEX(x, y, z)] = 3; // surface block
                else
                    this->blocks[CHUNK_INDEX(x, y, z)] = 0; // air
            }
        }
    }

    noise.destroy(&noise);
}




#endif
