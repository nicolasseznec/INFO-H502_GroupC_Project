#ifndef TEXTURE_H
#define TEXTURE_H

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

    /* (from branch `lights`)
    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int loadTexture(char const * path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
    */
};

#endif /* TEXTURE_H */