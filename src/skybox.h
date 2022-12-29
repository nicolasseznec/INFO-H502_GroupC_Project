#ifndef SKYBOX_H
#define SKYBOX_H

#include <iostream>
#include <map>

#include <glad/glad.h>

#include "shader.h"
#include "object.h"


class Skybox
{
public:
    Object cubeMap;
    GLuint cubeMapTexture;

    Skybox(
        std::string path, 
        std::map<std::string, GLenum> faces,
        const char* cubePath,
        Shader& shader
    ) : cubeMap(cubePath) {
        glGenTextures(1, &cubeMapTexture);
        bindTexture();

        // texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //stbi_set_flip_vertically_on_load(true);

        std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/yokohama3/";

        //load the six faces
        for (std::pair<std::string, GLenum> pair : faces) {
            loadCubemapFace((path + pair.first).c_str(), pair.second);
        }
	    cubeMap.makeObject(shader);
    }

    void loadCubemapFace(const char* path, const GLenum& targetFace)
    {
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {

            glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //glGenerateMipmap(targetFace);
        }
        else {
            std::cout << "Failed to Load texture at : " << path << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
        }
        stbi_image_free(data);
    }

    void bindTexture() 
    {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
    }

    void draw() 
    {
		cubeMap.draw();
    }

};

#endif /* SKYBOX_H */