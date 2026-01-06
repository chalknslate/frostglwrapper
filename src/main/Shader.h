#ifndef SHADER_H_
#define SHADER_H_
    #include "Vector.h"
    #include "Matrix4.h"
    #include "Vec.h"
    #include <stdio.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    #define GL_MAT4 0x1929

    struct Program;
    struct Shader {
        GLint id;
        GLenum type;
        void(*ld)(struct Shader* this, struct Program* program, char* filepath);
    };
    struct Program {
        GLuint value;
        struct Vector vector;
        int(*getProgramID)(struct Program* this);
        void(*destroy)(struct Program* this);
        void(*push_back)(struct Program* this, struct Shader* shader);
        void (*start)(struct Program* this);
        void (*stop)();
    };
    static void push(struct Program* this, struct Shader* shader) {
        this->vector.push_back(&this->vector, &shader->id);
    }
    static void start(struct Program* this) {
        glUseProgram(this->getProgramID(this));
    }

    // Function to stop using the shader program
    static void stop() {
        glUseProgram(0);
    }
    static int getProgramID(struct Program* this) {
        return this->value;
    }
    static void destroyP(struct Program* this) {
        for(int i = 0; i<this->vector.size; i++) {
            GLuint shaderID; this->vector.pop(&this->vector, (void*)&shaderID); 
            glDetachShader(this->getProgramID(this), shaderID);
            glDeleteShader(shaderID);
        }
        glDeleteProgram(this->value);
        this->vector.destroy(&this->vector);
    }

    static struct Program newProgram() {
        return (struct Program) {
            .value =  glCreateProgram(),
            .vector =  Vector.new(0, FIELD_TYPE_UINT),
            .getProgramID = &getProgramID,
            .destroy = &destroyP,
            .push_back = &push,
            .start = &start,
            .stop = &stop,
        };
    }

    static const struct {
        struct Program (*new)();
    } Program = { .new = &newProgram, };
    char* create_shader_content_from_file(const char* filename) {
        char *buffer = 0;
        long length;
        FILE *f = fopen(filename, "r");

        if (f) {
            fseek(f, 0, SEEK_END);
            length = ftell(f);
            fseek(f, 0, SEEK_SET);
            buffer = (char*)malloc(length + 1);  // +1 for null-terminator
            if (buffer) {
                fread(buffer, 1, length, f);
                buffer[length] = '\0';  // Ensure null-terminated string
            }
            fclose(f);
        }

        return buffer;
    }

    static void ld(struct Shader* this, struct Program* program, char* filepath) {
        char* shader = create_shader_content_from_file(filepath);
        const char* const *source = (const char* const*) &shader;
        int programID = program->getProgramID(program);
        glShaderSource(this->id, 1, source, NULL);
        glCompileShader(this->id);
        glValidateProgram(programID);
        glAttachShader(programID, this->id);
        glLinkProgram(programID);
        GLint success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLength;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
            char* log = malloc(500);
            glGetProgramInfoLog(programID, logLength, &logLength, log);
            printf("Program linking failed: %d\n%s\n", this->type, log);
            printf("CONTENTS: \n%s\n\n", shader);
            free(log);
        }
        program->push_back(program, this);
        free(shader);
    }
    inline static struct Shader newShader(GLenum type) {
        return (struct Shader) {
            .id = glCreateShader(type),
            .type = type,
            .ld = &ld,
        };
    }

    static const struct {
        struct Shader (*new)(GLenum type);
    } Shader = { .new = &newShader };

    struct Uniform {
        GLuint type;
        GLuint location;
        void (*ld)(struct Uniform* u, void* data);
    };
    void ldm(struct Uniform* u, void* data) {
        if(u->type == GL_MAT4) {
            struct Mat4 mat = *(struct Mat4*)data;
            glUniformMatrix4fv(u->location, 1, GL_TRUE, mat.m);
        } else if(u->type == GL_V3F) {
            struct Vec3 vec = *(struct Vec3*)data;
            struct Vector vals = Vector.new(0, FIELD_TYPE_FLOAT);
            vals.push_back(&vals, &vec.x);
            vals.push_back(&vals, &vec.y);
            vals.push_back(&vals, &vec.z);
            glUniform3fv(u->location, 1, (float*)vals.data);
            vals.destroy(&vals);
        }
    }
    inline static struct Uniform newUniform(GLuint type, struct Program* program, char* name) {
        return (struct Uniform) {
            .type = type,
            .location = glGetUniformLocation(program->getProgramID(program), name),
            .ld = &ldm,
        };
    }
    
    static const struct {
        struct Uniform (*new)(GLenum type, struct Program* program, char* name);
    } Uniform = { .new = &newUniform };
    void load_basics(struct Program* prog) {
        struct Mat4 mat = Mat4.new();
        struct Uniform proj = Uniform.new(GL_MAT4, prog, "proj");
        mat.projection(&mat, 70.0f, 640.0f/480.0f, 0.1f, 1000.0f);


        struct Mat4 mmodel = Mat4.new();
        struct Uniform umodel = Uniform.new(GL_MAT4, prog, "model");
        mmodel.transform(&mmodel, 0,0,0,0,0,0,1,1,1);

        
        prog->start(prog);
        proj.ld(&proj, (void*)&mat);
        umodel.ld(&umodel, (void*)&mmodel);
        prog->stop();
    }

    void load_cam(struct Program* prog, struct Vec3*pos, struct Vec3* rot) {
        struct Mat4 view = Mat4.new();
        struct Uniform mview = Uniform.new(GL_MAT4, prog, "view");
        view.view(&view, pos->x, pos->y, pos->z, rot->x, rot->y, rot->z);
        prog->start(prog);
        mview.ld(&mview, (void*)&view);
        prog->stop();
    }

#endif