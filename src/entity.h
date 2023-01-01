#ifndef ENTITY_H
#define ENTITY_H

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

    Texture* texture;
    Mesh* model;

    // TODO : proper model and texture management
    Entity(Mesh& model, Texture& texture) {
        this->model = &model;
        this->texture = &texture;
    }


    void draw(Shader& shader) {
        // If (Shader.current != shader.ID) Either error, return, or shader.use()

        /* TODO: with multiple textures:
        for each texture, depending on its type, choose right uniform name (e.g. texture_color_i, texture_normal_i, ...)
        shader.setInteger("name", i)
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
        */

        shader.setInteger("u_texture", 0);  // Set the texture unit to use (set with GL_TEXTURE0, GL_TEXTURE1, ...) (by default 0) (Could be done before the loop)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->ID);
		
        shader.setMatrix4("M", transform);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(transform)));
		model->draw();
	}

};

#endif /* ENTITY_H */