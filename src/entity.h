#ifndef ENTITY_H
#define ENTITY_H

// Some parts of the code were taken from https://learnopengl.com/

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"
#include "shader.h"
#include "texture.h"

class Entity 
{
public:
    glm::mat4 transform = glm::mat4(1.0);

    std::vector<Texture> textures;
    Mesh* model;

    Entity(Mesh& model, Texture texture) {
        this->model = &model;
        textures.push_back(texture);
    }
    
    Entity(Mesh& model, Texture texture, Texture normalMap) {
        this->model = &model;
        textures.push_back(texture);
        normalMap.type = NORMAL;
        textures.push_back(normalMap);
    }

    void draw(Shader& shader) {
        if (!model) return;
        bool useNormalMap = false;

        for (unsigned int i = 0; i < textures.size(); i++) {
            Texture& texture = textures[i];

            glActiveTexture(GL_TEXTURE0 + i); 
            glBindTexture(GL_TEXTURE_2D, texture.ID);

            if (texture.type == COLOR) {
                shader.setInteger("u_texture", i);
            }
            else if (texture.type == NORMAL) {
                useNormalMap = true;
                shader.setBool("useNormalMap", true);
                shader.setInteger("u_normalMap", i);
            }
        }

        shader.setMatrix4("M", transform);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(transform)));
		model->draw();
        
        if (useNormalMap) {
            shader.setBool("useNormalMap", false); // reset useNormalMap
        }
	}
};

#endif /* ENTITY_H */