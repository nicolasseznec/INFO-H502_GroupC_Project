#ifndef WINDOW_H
#define WINDOW_H

#include "entity.h"
#include "skybox.h"

class RoomWindow : public Entity
{
public:
    Skybox* skybox;

    RoomWindow(Mesh& model, Texture texture, Skybox* skybox = nullptr) : Entity(model, texture), skybox(skybox) {
    }

    void draw(Shader& shader) {
        shader.setInteger("cubemapSampler", 1);	
        if (skybox) skybox->bindTexture(1);
        Entity::draw(shader);
	}

};

#endif /* WINDOW_H */