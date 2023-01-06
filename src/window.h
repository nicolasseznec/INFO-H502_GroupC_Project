#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"


class Window : public Entity
{
public:

    // TODO : proper model and texture management
    Window(Mesh& model, Texture texture) : Entity(model, texture) {
    }

    void draw(Shader& shader) {
        Entity::draw(shader);
	}

};

#endif /* WINDOW_H */