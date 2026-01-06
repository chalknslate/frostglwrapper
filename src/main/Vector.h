#ifndef VECTOR_H_
#define VECTOR_H_
    #include "Vec.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    struct EKey {
        int v, vt, vn;
    };
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    typedef enum DATA_TYPE {
        DATA_TYPE_GLUINT,
        DATA_TYPE_FLOAT,
        DATA_TYPE_INT,
        DATA_TYPE_VEC3,
        DATA_TYPE_VEC2,
    } data_type;
    
    
    typedef enum field_type {
        FIELD_TYPE_BYTE,
        FIELD_TYPE_KEY,
        FIELD_TYPE_INT,
        FIELD_TYPE_FLOAT,
        FIELD_TYPE_UINT,
        FIELD_TYPE_VEC3,
        FIELD_TYPE_VEC2,
    } field_type;

    struct Vector {
        void* data;
        int size;
        int capacity;
        size_t elem_size;
        field_type type;

        void (*push_back)(struct Vector* this, void* value);
        void (*print_all)(struct Vector* this);
        void (*destroy)(struct Vector* this);
        int  (*pop)(struct Vector* this, void* out_value);
    };

    static void destroyArr(struct Vector* this) {
        free(this->data);
        this->data = NULL;
        this->size = 0;
        this->capacity = 0;
    }

    static void print_all(struct Vector* this) {
        for (int i = 0; i < this->size; i++) {
            if (this->type == FIELD_TYPE_INT) {
                printf("%d\n", ((int*)this->data)[i]);
            } else if (this->type == FIELD_TYPE_FLOAT) {
                printf("%f\n", ((float*)this->data)[i]);
            } else if (this->type == FIELD_TYPE_BYTE) {
                printf("%u\n", ((unsigned char*)this->data)[i]);
            } else if (this->type == FIELD_TYPE_UINT) {
                printf("%u\n", ((unsigned int*)this->data)[i]);
            }
        }
    }

    static void push_back(struct Vector* this, void* value) {
        if (this->size >= this->capacity) {
            int new_capacity = (this->capacity == 0) ? 1 : this->capacity * 2;
            this->data = realloc(this->data, new_capacity * this->elem_size);
            this->capacity = new_capacity;
        }
        memcpy((char*)this->data + this->size * this->elem_size, value, this->elem_size);
        this->size++;
    }

    static size_t get_elem_size(field_type type) {
        switch (type) {
            case FIELD_TYPE_INT: return sizeof(int);
            case FIELD_TYPE_FLOAT: return sizeof(float);
            case FIELD_TYPE_BYTE: return sizeof(unsigned char);
            case FIELD_TYPE_VEC3: return sizeof(struct Vec3);
            case FIELD_TYPE_VEC2: return sizeof(struct Vec2);
            case FIELD_TYPE_KEY: return sizeof(struct EKey);
            default: return 1;
        }
    }

    static int pop(struct Vector* this, void* out_value) {
        if (this->size == 0) {
            return 0; // nothing to pop
        }

        this->size--;

        // copy last element into out_value
        memcpy(out_value, (char*)this->data + this->size * this->elem_size, this->elem_size);

        return 1; // success
    }

    static struct Vector newArr(int cap, field_type type) {
        size_t elem_size = get_elem_size(type);
        return (struct Vector){
            .data = (cap > 0) ? malloc(elem_size * cap) : NULL,
            .size = 0,
            .capacity = cap,
            .elem_size = elem_size,
            .type = type,
            .push_back = &push_back,
            .print_all = &print_all,
            .destroy = &destroyArr,
            .pop = &pop,
        };
    }


    static const struct {
        struct Vector (*new)(int cap, field_type type);
    } Vector = { .new = &newArr };

#endif
