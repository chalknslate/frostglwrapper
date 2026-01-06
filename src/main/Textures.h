#ifndef TEXTURES_H_
#define TEXTURES_H_
    #include <stdio.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    #include "stb_image.h"
    struct Texture {
        unsigned int id;
        int slot;
        GLuint (*getID)(struct Texture* this);
        void (*use)(struct Texture* this);
    };
    GLuint getTextureID(struct Texture* this) {
        return this->id;
    }
    void use(struct Texture* this) {
        glActiveTexture(GL_TEXTURE0 /*+ this->slot*/);
        glBindTexture(GL_TEXTURE_2D, this->getID(this));
    }

    struct Texture newTexture(char* imagepath, int slot) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(1);
        unsigned char *data = stbi_load(imagepath, &width, &height, &nrChannels, STBI_default);
        
        if (!data) {
            printf("Failed to load texture: %s\n", imagepath);
        }

        printf("Loaded texture: %s | Width: %d, Height: %d, Channels: %d\n", imagepath, width, height, nrChannels);

        GLuint textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0+slot);
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // fallback

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    
        
        // Set texture parameters
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);  // Free image memory after uploading to OpenGL
        return (struct Texture) {
            .id = textureID,
            .slot = slot,
            .getID = &getTextureID,
            .use = &use,
        };
        
    }
    static const struct {
        struct Texture (*new)(char* imagepath, int slot);
    } Texture = { .new = &newTexture };

#endif