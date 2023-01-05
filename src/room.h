#ifndef ROOM_H
#define ROOM_H

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "entity.h"

class RoomScene
{
public:
    Mesh room_mesh = Mesh(PATH_TO_OBJECTS "/room/room.obj");
    Mesh carpet_mesh = Mesh(PATH_TO_OBJECTS "/room/carpet.obj");
    Mesh bench_mesh = Mesh(PATH_TO_OBJECTS "/room/bench.obj");

    std::vector<Entity> objects; 

    glm::mat4 transform = glm::mat4(1.0);

    RoomScene() {
        objects.push_back(Entity(room_mesh, Texture(PATH_TO_TEXTURE "/room/room_colormap.jpg")));
        objects.push_back(Entity(carpet_mesh, Texture(PATH_TO_TEXTURE "/room/carpet_colormap.jpg")));
        objects.push_back(Entity(bench_mesh, Texture(PATH_TO_TEXTURE "/room/bench_colormap.jpg")));

        transform = glm::translate(transform, glm::vec3(0.0f, -1.0f, -2.0f));
        for (Entity& object : objects) {
            object.transform = this->transform * object.transform;
        }
    }

    void draw(Shader& shader) {
        for (Entity& object : objects) {
            object.draw(shader);
        }
    }
    
};

#endif /* ROOM_H */