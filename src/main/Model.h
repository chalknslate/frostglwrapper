#ifndef MODEL_H_
#define MODEL_H_
    #include "Vec.h"
    #include "Vector.h"
    #include <stdio.h>
    #include <stdlib.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    typedef enum {
        ENG_INT = 1,
        ENG_VEC3 = 2,
        ENG_VEC2 = 3,
    } ModelDataType;
    static struct {
        struct Vector VAOS;
        struct Vector VBOS;
    } ModelDataInitializer;
    struct ModelDataInfo {
        void* data;
        ModelDataType type;
        int count;
    };

    int store_attrib_data(int position, int coordinateSize, struct ModelDataInfo* info) {
        GLuint vboID;
        glGenBuffers(1, &vboID);
        ModelDataInitializer.VBOS.push_back(&ModelDataInitializer.VBOS, &vboID);
        switch(info->type) {
            case ENG_VEC3: {
                float* dat = (float*)malloc((info->count*3)*sizeof(float));
                struct Vec3* pdata = (struct Vec3*)(info->data);
                for(int i = 0; i<info->count; i++) {
                    //printf("vert: %f, %f, %f\n",pdata[i].getX(&pdata[i]), pdata[i].getY(&pdata[i]), pdata[i].getZ(&pdata[i]));
                    dat[(i*3)+0] = pdata[i].getX(&pdata[i]);
                    dat[(i*3)+1] = pdata[i].getY(&pdata[i]);
                    dat[(i*3)+2] = pdata[i].getZ(&pdata[i]);
                }
                glBindBuffer(GL_ARRAY_BUFFER, vboID);
                glBufferData(GL_ARRAY_BUFFER, (info->count*3) * sizeof(float), dat, GL_STATIC_DRAW);
                glVertexAttribPointer(position, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                free(dat);
                break;
            }
            case ENG_INT: {
                int *data = (int*)(info->data);
                for(int i = 0; i<info->count/3; i++) {
                    //printf("indice (grouped into vec3) %d, %d, %d\n", data[(i*3)+0],data[(i*3)+1],data[(i*3)+2]);
                }
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, info->count * sizeof(int), data, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                break;
            }
            case ENG_VEC2: {
                float* dat = (float*)malloc((info->count*2)*sizeof(float));
                struct Vec2* pdata = (struct Vec2*)(info->data);
                for(int i = 0; i<info->count; i++) {
                   // printf("uv: %f, %f\n",pdata[i].getX(&pdata[i]), pdata[i].getY(&pdata[i]));
                    dat[(i*2)+0] = pdata[i].getX(&pdata[i]);
                    dat[(i*2)+1] = pdata[i].getY(&pdata[i]);
                }
                
                glBindBuffer(GL_ARRAY_BUFFER, vboID);
                glBufferData(GL_ARRAY_BUFFER, (info->count*2) * sizeof(float), dat, GL_STATIC_DRAW);
                glVertexAttribPointer(position, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                free(dat);
            }
        }
        return vboID;
    }
    struct Model {
        int vaoID;
        GLuint iboID;
        int vertexCount;

        void* vertices;
        void* normals;
        void* uvs;
        void* indices;
        int indexCount;

        void(*ld)(struct Model* this, struct ModelDataInfo* v, struct ModelDataInfo* i,struct ModelDataInfo* uv,struct ModelDataInfo* n);
    };
    static void ldmd(struct Model* this, struct ModelDataInfo* v, struct ModelDataInfo* i, struct ModelDataInfo* uv,struct ModelDataInfo* n) {
        GLuint vaoID;
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);
        GLuint vertexData = store_attrib_data(0,3,v);
        GLuint uvData = store_attrib_data(1,2,uv);
        GLuint nData = store_attrib_data(2,3,n);
        ModelDataInitializer.VAOS.push_back(&ModelDataInitializer.VAOS, &vaoID);
        GLuint iboID = store_attrib_data(0,0,i);
        this->vaoID = vaoID;
        this->iboID = iboID;
        //printf("%d\n", i->count);
        this->vertexCount = i->count;

        this->indexCount  = i->count;

    }
    inline static struct Model newModel() {
        return (struct Model) {
            .vaoID = 0,
            .iboID = 0,
            .vertexCount = 0,
            .ld = &ldmd,
        };
    }
    static const struct {
        struct Model (*new)();
    } Model = { .new = &newModel};


    inline static struct ModelDataInfo create(void* addr, ModelDataType type, int count) {
        return (struct ModelDataInfo) {
            .data = addr,
            .type = type,
            .count = count,
        };
    }

    static const struct {
        struct ModelDataInfo (*new)(void* addr, ModelDataType type, int count);
    } ModelDataInfo = { .new = &create};

    void initialize() {
        ModelDataInitializer.VAOS = Vector.new(0, FIELD_TYPE_UINT);
        ModelDataInitializer.VBOS = Vector.new(0, FIELD_TYPE_UINT);
    }
    void fr_exit() {
        for(int i = 0; i<ModelDataInitializer.VBOS.size; i++) {
            glDeleteBuffers(1,&ModelDataInitializer.VBOS.data[i]);
        }
        for(int i = 0; i<ModelDataInitializer.VBOS.size; i++) {
            glDeleteVertexArrays(1, &ModelDataInitializer.VAOS.data[i]);
        }
        ModelDataInitializer.VAOS.destroy(&ModelDataInitializer.VAOS);
        ModelDataInitializer.VBOS.destroy(&ModelDataInitializer.VBOS);
    }
#endif