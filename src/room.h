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

    RoomScene() {
        objects.push_back(Entity(room_mesh, Texture(PATH_TO_TEXTURE "/room/room_colormap.jpg")));
        objects.push_back(Entity(carpet_mesh, Texture(PATH_TO_TEXTURE "/room/carpet_colormap.jpg")));
        objects.push_back(Entity(bench_mesh, Texture(PATH_TO_TEXTURE "/room/bench_colormap.jpg")));
    }

    void draw(Shader& shader) {
        for (Entity& object : objects) {
            object.draw(shader);
        }
    }
    
};

#endif /* ROOM_H */