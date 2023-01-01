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


class Entity 
{
public:
    glm::mat4 transform = glm::mat4(1.0);

    GLuint texture;
    Mesh* model;

    // TODO : proper model and texture management
    Entity(Mesh& model, GLuint texture) {
        this->model = &model;
        this->texture = texture;
    }


    void draw(Shader& shader) {
        // If (Shader.current != shader.ID) Either error, return, or shader.use()

        shader.setInteger("u_texture", 0);  // Set the texture unit to use (set with GL_TEXTURE0, GL_TEXTURE1, ...) (by default 0) (Could be done before the loop)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
        shader.setMatrix4("M", transform);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(transform)));
		model->draw();
	}

};

#endif /* ENTITY_H */