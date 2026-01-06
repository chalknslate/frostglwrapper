/*
#ifndef LOADEDMODEL_H_
#define LOADEDMODEL_H_

    #include "Integer.h"
    #include "Vec.h"
    #include "Window.h"
    #include "Shader.h"
    #include "Camera.h"
    #include "Program.h"
    #include "Renderer.h"
    #include "Vector.h"
    #include "Model.h"
    #include "Matrix4.h"
    #include <stdio.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    struct LoadedModel {
        struct Model model;
    };

    static struct LoadedModel loadOBJ(const char* path) {
        FILE* file = fopen(path, "r");
        if (!file) {
            fprintf(stderr, "Could not open OBJ file: %s\n", path);
            exit(1);
        }

        struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
        struct Vector uvs = Vector.new(0, FIELD_TYPE_VEC2);
        struct Vector indices = Vector.new(0, FIELD_TYPE_INT);


        struct Vector reordereduvs = Vector.new(0,FIELD_TYPE_VEC2);
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "v ", 2) == 0) {
                float x, y, z;
                sscanf(line, "v %f %f %f", &x, &y, &z);
                struct Vec3 v = Vec3.new(x, y, z);
                vertices.push_back(&vertices, &v); 
            } else if (strncmp(line, "vt ", 3) == 0) {
                float u, v;
                sscanf(line, "vt %f %f", &u, &v);
                struct Vec2 uv = Vec2.new(u, 1-v);
                uvs.push_back(&uvs, &uv);
            } else if (strncmp(line, "f ", 2) == 0) {
                int vIdx[3], uvIdx[3], nIdx[3];
                // Parse format: f v1/vt1 v2/vt2 v3/vt3
                sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                    &vIdx[0], &uvIdx[0], &nIdx[0],
                    &vIdx[1], &uvIdx[1], &nIdx[1],
                    &vIdx[2], &uvIdx[2], &nIdx[2]);

                // OBJ indices are 1-based
                for (int i = 0; i < 3; i++) {
                    int value = (int)(vIdx[i] - 1);
                    indices.push_back(&indices, &value);
                }
                for(int i = 0; i<3; i++) {
                    struct Vec2* uvData = (struct Vec2*)uvs.data;
                    struct Vec2 uv = (uvData[uvIdx[i]-1]);
                    printf("reordered UV ID:%d, (%f,%f)\n", uvIdx[i]-1,uv.x, uv.y);
                    reordereduvs.push_back(&reordereduvs, &(uvData[uvIdx[i]-1]));
                }
            }
        }
        fclose(file);

        // Wrap into ModelDataInfo
        struct ModelDataInfo vInfo = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
        struct ModelDataInfo uvInfo = ModelDataInfo.new(reordereduvs.data, ENG_VEC2, reordereduvs.size);
        struct ModelDataInfo iInfo = ModelDataInfo.new(indices.data, ENG_INT, indices.size);

        // Create Model
        struct Model model = Model.new();
        model.ld(&model, &vInfo, &iInfo, &uvInfo);

        // Cleanup CPU-side arrays
        vertices.destroy(&vertices);
        uvs.destroy(&uvs);
        reordereduvs.destroy(&reordereduvs);
        indices.destroy(&indices);
        return (struct LoadedModel){ .model = model };
    }

    static const struct {
        struct LoadedModel (*new)(const char* path);
    } LoadedModel = { .new = &loadOBJ };



#endif

#ifndef LOADEDMODEL_H_
#define LOADEDMODEL_H_

#include "Vec.h"
#include "Vector.h"
#include "Model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LoadedModel {
    struct Model model;
};

static struct LoadedModel loadOBJ(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not open OBJ file: %s\n", path);
        exit(1);
    }

    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector indices  = Vector.new(0, FIELD_TYPE_INT);
    struct Vector uvs = Vector.new(0, FIELD_TYPE_VEC2);
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            struct Vec3 v = Vec3.new(x, y, z);
            vertices.push_back(&vertices, &v);
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            struct Vec2 uv = Vec2.new(u,  v); // flip V
            uvs.push_back(&uvs, &uv);
        } else if (strncmp(line, "f ", 2) == 0) {
            int vIdx[3], uvIdx[3], nIdx[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vIdx[0], &uvIdx[0], &nIdx[0],
                &vIdx[1], &uvIdx[1], &nIdx[1],
                &vIdx[2], &uvIdx[2], &nIdx[2]);
            for (int i = 0; i < 3; i++) {
                int idx = vIdx[i] - 1; // OBJ indices are 1-based
                indices.push_back(&indices, &idx);
            }
        }
    }
    fclose(file);

    struct ModelDataInfo vInfo = ModelDataInfo.new(vertices.data, ENG_VEC3, vertices.size);
    struct ModelDataInfo iInfo = ModelDataInfo.new(indices.data, ENG_INT, indices.size);
    struct ModelDataInfo uInfo = ModelDataInfo.new(uvs.data, ENG_VEC2, uvs.size);

    struct Model model = Model.new();
    model.ld(&model, &vInfo, &iInfo, &uInfo); // UVs NULL

    vertices.destroy(&vertices);
    indices.destroy(&indices);

    return (struct LoadedModel){ .model = model };
}

static const struct {
    struct LoadedModel (*new)(const char* path);
} LoadedModel = { .new = &loadOBJ };

#endif


#ifndef LOADEDMODEL_H_
#define LOADEDMODEL_H_
#include "Integer.h"
    #include "Vec.h"
    #include "Window.h"
    #include "Shader.h"
    #include "Camera.h"
    #include "Program.h"
    #include "Renderer.h"
    #include "Vector.h"
    #include "Model.h"
    #include "Matrix4.h"
    #include <stdio.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
int is_empty_or_whitespace(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;  // Found a non-whitespace character
        }
        str++;
    }
    return 1;  // Only whitespace or empty
}

// Convert string to float
int get_int(char* s) {
    char *endptr;
    long int num;
    int sign = 0;
    num = strtol(s, &endptr, 10);
    if(num<0) {
        sign = -1;
    }
    if(num > INT_MAX) {
        return (num - (num-1))*INT_MAX;
    }else if (num < INT_MAX*-1) {
        return ((num - (num-1))*INT_MAX+1)*sign;
    } else {
        return num;
    }
}
float gn(char *line) {
    if (line == NULL) return 0.0f;  // Handle NULL input safely

    char *endptr;
    float val = strtof(line, &endptr);  // Convert string to float

    if (line == endptr) {
        // No valid float found in the string
        return 0.0f;
    }

    return val;
}
#include "Vec.h"
#include "Vector.h"
#include "Model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LoadedModel {
    struct Model model;
};
static struct LoadedModel loadOBJ(const char* path) {
    char *buffer = 0;
    long length;
    FILE *f = fopen(path, "r");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);  // +1 for null-terminator
        if (buffer) {
            fread(buffer, 1, length, f);
            buffer[length] = '\0';  // Ensure null-terminated string
        }
        fclose(f);
    } else {
        printf("File not found\n");
    }

    // Dynamic allocation for tokens
    char **tokens = NULL;
    int nl = 0;  // Number of non-empty lines processed
    struct Vector textureCoords = Vector.new(0,FIELD_TYPE_VEC2);
    struct Vector verts = Vector.new(0,FIELD_TYPE_VEC3);
    struct Vector texts = Vector.new(0,FIELD_TYPE_VEC2);
    struct Vector indices = Vector.new(0,FIELD_TYPE_INT);
    struct Vector rtexts = Vector.new(0,FIELD_TYPE_VEC2);

    // Tokenize the buffer into lines
    char* token = strtok(buffer, "\n");
    while (token != NULL) {
        if (!is_empty_or_whitespace(token)) {
            // Increment line count for valid non-empty lines
            nl++;
            tokens = realloc(tokens, nl * sizeof(char*));
            tokens[nl - 1] = strdup(token);  // Copy the token string to allocated memory
        }
        token = strtok(NULL, "\n");
    }
    // Debugging output: print number of non-empty lines
    //printf("Number of non-empty lines: %d\n", nl);

    // Process the tokens (lines)
    for (int i = 0; i < nl; i++) {
        if (tokens[i] == NULL) {
            printf("Error: tokens[%d] is NULL\n", i);  // Log the issue
            continue;  // Skip if token is NULL
        }

        // Now we are safe to call string functions like strncmp or strlen
        if (strncmp(tokens[i], "v ", 2) == 0) {
            char* coords = strtok(tokens[i], " ");
            char* x = strtok(NULL, " ");
            char* y = strtok(NULL, " ");
            char* z = strtok(NULL, " ");
            // Check if x, y, z are valid before using them
            if (x == NULL || y == NULL || z == NULL) {
                printf("Error: Invalid vertex data in tokens[%d]\n", i);
                continue;  // Skip invalid token
            }

            // Now we are safe to process the values
            float xv = gn(x);
            float yv = gn(y);
            float zv = gn(z);
            struct Vec3 vec = Vec3.new(xv,yv,zv);
            verts.push_back(&verts,&vec);
            // Debug output for vertex values
            //printf("Vertex %d: x=%f, y=%f, z=%f\n", i, xv, yv, zv);
        }
        if (strncmp(tokens[i], "vt ", 3) == 0) {
            char* coords = strtok(tokens[i], " ");
            char* x = strtok(NULL, " ");
            char* y = strtok(NULL, " ");

            // Check if x, y, z are valid before using them
            if (x == NULL || y == NULL) {
                printf("Error: Invalid vertex data in tokens[%d]\n", i);
                continue;  // Skip invalid token
            }

            // Now we are safe to process the values
            float xv = gn(x);
            float yv = gn(y);
            struct Vec2 uv = Vec2.new(xv,1-yv);
            textureCoords.push_back(&textureCoords, &uv);
            // Debug output for vertex values
            //printf("Texture Coord %d: x=%f, y=%f\n", i, xv, yv);
        }
        if (strncmp(tokens[i], "f ", 2) == 0) {
            int faceCount = 0;
            char **faces = NULL;
            strtok(tokens[i], " ");
            char* f1 = strtok(NULL, " ");
            while (f1 != NULL) {
                // Increment line count for valid non-empty lines
                faceCount++;
                faces = realloc(faces, faceCount * sizeof(char*));
                faces[faceCount - 1] = strdup(f1);  // Copy the token string to allocated memory
                f1 = strtok(NULL, " ");
            }
            for(int i = 0; i <faceCount; i++) {
                char* i1 = strtok(faces[i], "/");
                char *t1 = strtok(NULL, "/");
                int cvp = get_int(i1)-1;

                indices.push_back(&indices,&cvp);
                struct Vec2 uv = ((struct Vec2*)textureCoords.data)[get_int(t1)-1];
                rtexts.push_back(&rtexts, &uv);


                //printf("Texture Coord %d: (%f, %f), Index: (%d, %d),", i, textureCoords[get_int(t1)-1].x, 1-textureCoords[get_int(t1)-1].y, cvp*2, (cvp*2)+1);
            }
            for (int i = 0; i < faceCount; i++) {
                free(faces[i]);  // Free individual tokens
            }
            free(faces);
            //printf("\n");
        }
    }
    for (int i = 0; i < nl; i++) {
        free(tokens[i]);  // Free individual tokens
    }
    
    free(tokens); 
 // Free the tokens array
    free(buffer);  // Free the buffer
    struct ModelDataInfo vInfo = ModelDataInfo.new(verts.data, ENG_VEC3, verts.size);
    struct ModelDataInfo iInfo = ModelDataInfo.new(indices.data, ENG_INT, indices.size);
    struct ModelDataInfo uInfo = ModelDataInfo.new(rtexts.data, ENG_VEC2, rtexts.size);

    struct Model model = Model.new();
    model.ld(&model, &vInfo, &iInfo, &uInfo);

    verts.destroy(&verts);
    rtexts.destroy(&rtexts);
    texts.destroy(&texts);
    indices.destroy(&indices);
    textureCoords.destroy(&textureCoords);
    return (struct LoadedModel) {.model = model};
}
static const struct {
    struct LoadedModel (*new)(const char* path);
} LoadedModel = { .new = &loadOBJ };
#endif

#ifndef LOADEDMODEL_H_
#define LOADEDMODEL_H_

#include "Integer.h"
#include "Vec.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"
#include "Program.h"
#include "Renderer.h"
#include "Vector.h"
#include "Model.h"
#include "Matrix4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad.h>

// Composite key for deduplication
struct Key {
    int v, vt, vn;
};

// Compare helper for keys
static int keyEquals(struct Key a, struct Key b) {
    return a.v == b.v && a.vt == b.vt && a.vn == b.vn;
}

// Our final packed vertex
struct PackedVertex {
    struct Vec3 position;
    struct Vec2 uv;
    // normals could be added here if needed
};

struct LoadedModel {
    struct Model model;
};

static struct LoadedModel loadOBJ(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not open OBJ file: %s\n", path);
        exit(1);
    }

    // Raw OBJ data
    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);
    // (normals could go here if you want them)

    // Final deduplicated mesh
    struct Vector finalVertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector finalUVs =  Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector uvIndexes = Vector.new(0, FIELD_TYPE_INT);
    struct Vector finalIndices  = Vector.new(0, FIELD_TYPE_INT);
    struct Vector usedKeys      = Vector.new(0, FIELD_TYPE_KEY); // store Key[]

    char line[512];
    int uvID = 1;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            struct Vec3 v = Vec3.new(x, y, z);
            vertices.push_back(&vertices, &v);
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            struct Vec2 uv = Vec2.new(u,  1-v); // flip V
            uvs.push_back(&uvs, &uv);
            printf("UV ID:%d (%f, %f)\n",uvID,u,v);
            uvID++;
        } else if (strncmp(line, "f ", 2) == 0) {
            int vIdx[3], uvIdx[3], nIdx[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vIdx[0], &uvIdx[0], &nIdx[0],
                &vIdx[1], &uvIdx[1], &nIdx[1],
                &vIdx[2], &uvIdx[2], &nIdx[2]);

            for (int i = 0; i < 3; i++) {
                // get original data
                struct Vec3 pos = ((struct Vec3*)vertices.data)[vIdx[i] - 1];
                struct Vec2 uv  = ((struct Vec2*)uvs.data)[uvIdx[i] - 1];

                // push into final arrays in matching order
                finalVertices.push_back(&finalVertices, &pos);
                finalUVs.push_back(&finalUVs, &uv);

                int newIndex = finalVertices.size - 1;
                finalIndices.push_back(&finalIndices, &newIndex);
            }
        }
    }

    fclose(file);

    struct ModelDataInfo vInfo = ModelDataInfo.new(finalVertices.data, ENG_VEC3, finalVertices.size);
    struct ModelDataInfo uvInfo = ModelDataInfo.new(finalUVs.data, ENG_VEC2, finalUVs.size);
    struct ModelDataInfo iInfo = ModelDataInfo.new(finalIndices.data, ENG_INT, finalIndices.size);

    // Create Model
    struct Model model = Model.new();
    model.ld(&model, &vInfo, &iInfo, &uvInfo);

    // Cleanup CPU-side arrays
    vertices.destroy(&vertices);
    uvs.destroy(&uvs);
    finalVertices.destroy(&finalVertices);
    finalUVs.destroy(&finalUVs);
    finalIndices.destroy(&finalIndices);
    usedKeys.destroy(&usedKeys);

    return (struct LoadedModel){ .model = model };
}

static const struct {
    struct LoadedModel (*new)(const char* path);
} LoadedModel = { .new = &loadOBJ };

#endif
*/

#ifndef LOADEDMODEL_H_
#define LOADEDMODEL_H_

#include "Integer.h"
#include "Vec.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"
#include "Program.h"
#include "Renderer.h"
#include "Vector.h"
#include "Model.h"
#include "Matrix4.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad.h>

struct LoadedModel {
    struct Model model;
    struct Vec3 pos;
    struct Vec3 rot;
    void (*use)(struct LoadedModel* this, struct Program *prog);
};
void useMdl(struct LoadedModel* this, struct Program *prog) {
    struct Mat4 mmodel = Mat4.new();
    struct Uniform umodel = Uniform.new(GL_MAT4, prog, "model");
    mmodel.transform(&mmodel, this->pos.x,this->pos.y, this->pos.z,this->rot.x,this->rot.y,this->rot.z,0.5,0.5,0.5);
    umodel.ld(&umodel, (void*)&mmodel);
}
static struct LoadedModel loadOBJ(const char* path, struct Vec3* pos, struct Vec3 *rot) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not open OBJ file: %s\n", path);
        exit(1);
    }

    // Raw OBJ data
    struct Vector vertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector normals  = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector uvs      = Vector.new(0, FIELD_TYPE_VEC2);

    // Final expanded arrays
    struct Vector finalVertices = Vector.new(0, FIELD_TYPE_VEC3);
    struct Vector finalUVs      = Vector.new(0, FIELD_TYPE_VEC2);
    struct Vector finalIndices  = Vector.new(0, FIELD_TYPE_INT);
    struct Vector finalNormals  = Vector.new(0, FIELD_TYPE_VEC3);
    char line[512];
    int uvID = 1;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            struct Vec3 v = Vec3.new(x, y, z);
            vertices.push_back(&vertices, &v);
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            struct Vec2 uv = Vec2.new(u, v); // flip V
            uvs.push_back(&uvs, &uv);
            // Debugging (optional):
            // printf("UV ID:%d (%f, %f)\n", uvID, u, v);
            uvID++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            sscanf(line, "vn %f %f %f", &x, &y, &z);
            struct Vec3 v = Vec3.new(x, y, z);
            normals.push_back(&normals, &v);
        } else if (strncmp(line, "f ", 2) == 0) {
            int vIdx[3], uvIdx[3], nIdx[3];
            int count = sscanf(line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vIdx[0], &uvIdx[0], &nIdx[0],
                &vIdx[1], &uvIdx[1], &nIdx[1],
                &vIdx[2], &uvIdx[2], &nIdx[2]);

            if (count == 9) {
                for (int i = 0; i < 3; i++) {
                    struct Vec3 pos = ((struct Vec3*)vertices.data)[vIdx[i] - 1];
                    struct Vec3 nml = ((struct Vec3*)normals.data)[nIdx[i] - 1];
                    struct Vec2 uv  = ((struct Vec2*)uvs.data)[uvIdx[i] - 1];
                    //printf("nml ;; (%f, %f, %f)\n", nml.x, nml.y, nml.z);
                    finalVertices.push_back(&finalVertices, &pos);
                    finalUVs.push_back(&finalUVs, &uv);
                    finalNormals.push_back(&finalNormals, &nml);
                    int newIndex = finalVertices.size - 1;
                    finalIndices.push_back(&finalIndices, &newIndex);
                }
            }
        }
    }
    fclose(file);

    struct ModelDataInfo vInfo  = ModelDataInfo.new(finalVertices.data, ENG_VEC3, finalVertices.size);
    struct ModelDataInfo uvInfo = ModelDataInfo.new(finalUVs.data,      ENG_VEC2,      finalUVs.size);
    struct ModelDataInfo iInfo  = ModelDataInfo.new(finalIndices.data,  ENG_INT,   finalIndices.size);
    struct ModelDataInfo nInfo  = ModelDataInfo.new(finalNormals.data,  ENG_VEC3,  finalNormals.size);
    struct Model model = Model.new();
    model.ld(&model, &vInfo, &iInfo, &uvInfo,&nInfo);

    // Cleanup
    vertices.destroy(&vertices);
    uvs.destroy(&uvs);
    finalNormals.destroy(&finalNormals);
    finalVertices.destroy(&finalVertices);
    finalUVs.destroy(&finalUVs);
    finalIndices.destroy(&finalIndices);

    return (struct LoadedModel){ .model = model, .pos = *pos, .rot = *rot, .use = &useMdl};
}

static const struct {
    struct LoadedModel (*new)(const char* path, struct Vec3 *pos, struct Vec3 *rot);
} LoadedModel = { .new = &loadOBJ };

#endif
