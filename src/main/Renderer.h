#ifndef RENDERER_H_
#define RENDERER_H_
    #include "Model.h"
    #include "Textures.h"
    #include "Chunk.h"
    #include "World.h"
    struct Renderer {
        void(*render)(struct Model* model, struct Texture* texture);
        void(*renderChunk)(struct Chunk* chunk, struct Program* program);
        void(*renderWorld)(struct World* world, struct Program* program);
    };

    void render(struct Model* model, struct Texture* texture) {
        glBindVertexArray(model->vaoID);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->iboID);
        glDrawElements(GL_TRIANGLES, model->vertexCount, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }
     void renderChunk(struct Chunk* chunk, struct Program* program) {
        struct Model* model = &chunk->mesh;
        struct Mat4 mmodel = Mat4.new();
        struct Uniform umodel = Uniform.new(GL_MAT4, program, "model");
        mmodel.transform(&mmodel, chunk->position->x*CHUNK_SIZE, 0,chunk->position->y*CHUNK_SIZE,0,0,0,1,1,1);
        umodel.ld(&umodel, (void*)&mmodel);
        glBindVertexArray(model->vaoID);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->iboID);
        glDrawElements(GL_TRIANGLES, model->vertexCount, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
     }
     
     void renderWorld(struct World* world, struct Program* program) {
        for (int z = 0; z < world->depth; z++) {
            for (int x = 0; x < world->width; x++) {
                struct Chunk* chunk = World_getChunk(world, x, z);
                renderChunk(chunk, program);
            }
        }
    }

    inline static struct Renderer newRenderer() {
        return (struct Renderer) {
            .render = &render,
            .renderChunk = &renderChunk,
            .renderWorld = &renderWorld,
        };
    }

    static const struct {
        struct Renderer (*new)();
    } Renderer = {.new = &newRenderer, };


#endif