#ifndef TEXTURE_H
#define TEXTURE_H

// Some parts of the code were taken from https://learnopengl.com/


#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum TextureType {
    COLOR,
    NORMAL,
};


class Texture 
{
public:
    GLuint ID;
    TextureType type;

    Texture(const char* path, TextureType type = COLOR) {
        ID = loadTexture(path);
        this->type = type;
    }


    GLuint loadTexture(const char* path) {
        GLuint texture;
        glGenTextures(1, &texture);
        // glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        stbi_set_flip_vertically_on_load(true);
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {
            GLenum format;
            if (imNrChannels == 1)
                format = GL_RED;
            else if (imNrChannels == 3)
                format = GL_RGB;
            else if (imNrChannels == 4)
                format = GL_RGBA;


            // std::cout << "Loaded texture : " << path << " | " << imWidth << " * " << imHeight << std::endl;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // in case the texture is in non-power-of-two
            glTexImage2D(GL_TEXTURE_2D, 0, format, imWidth, imHeight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
        }
        stbi_set_flip_vertically_on_load(false);
        stbi_image_free(data);

        return texture;
    }
};

#endif /* TEXTURE_H */