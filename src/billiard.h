#ifndef BILLIARD_H
#define BILLIARD_H

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "mesh.h"
#include "entity.h"
#include "ball.h"


struct PoolRail {

};

class PoolGame 
{
public:
    Mesh tableMesh;
    Mesh ballMesh;

    Entity table;
    std::vector<PoolBall> balls;

    // TODO : proper model and texture management
    PoolGame(
        const char* tableMeshPath,
        const char* tableTexturePath,
        const char* ballMeshPath,
        const char* ballTexturePath,
        std::vector<std::string> ballTextureNames
        ) : tableMesh(tableMeshPath), table(tableMesh, Texture(tableTexturePath)), ballMesh(ballMeshPath) {
        
        table.transform = glm::translate(table.transform, glm::vec3(0.0, -1.0, -2.0));

        for (std::string name : ballTextureNames) {
            Texture texture = Texture(name.c_str());
            std::cout << "Loaded texture " << name << " to "<< texture.ID << std::endl;
            balls.push_back(PoolBall(ballMesh, texture));
        }

        for (int i = 0; i < balls.size(); i++) {
            balls.at(i).transform = glm::translate(balls.at(i).transform, glm::vec3(-0.5 + (i * 0.2), 0.0, -2.0));
        }

        std::cout << "Loaded " << balls.size() << " balls " << std::endl;
    }    

    void update(double deltaTime) {
        for (PoolBall ball : balls) {
            ball.update(deltaTime);
        }

        for (int i = 0; i < balls.size(); i++) {
            for (int j = i+1; j < balls.size(); j++) {
                if (balls.at(i).checkCollision(balls.at(j))) {
                    balls.at(i).handleCollision(balls.at(j));
                }
            }
        }
    }

    // TODO : separate shaders for table and balls ?
    void draw(Shader& shader) {
        table.draw(shader);
        
        for (PoolBall ball : balls) {
            ball.draw(shader);
        }
    }

};


#endif /* BILLIARD_H */