#ifndef BILLIARD_H
#define BILLIARD_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "mesh.h"
#include "entity.h"
#include "ball.h"



const glm::vec3 TABLE_DIM = glm::vec3(1.92f, 0.986f, 0.96f); //TODO : measure
const glm::vec3 COORD_RES = glm::vec3(200.0f, 1.0f, 100.0f);

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
        std::string ballTexturePath
        ) : tableMesh(tableMeshPath), table(tableMesh, Texture(tableTexturePath)), ballMesh(ballMeshPath) {
        
        // for (int i = 0; i < 16; i++) {
        for (int i = 0; i < 11; i++) {
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << i;
            Texture texture = Texture((ballTexturePath + "ball_" + ss.str() + ".jpg").c_str()); // TODO : not hardcoding the balls name ?
            balls.push_back(PoolBall(ballMesh, texture));
        }

        table.transform = glm::translate(table.transform, glm::vec3(0.0, -1.0, -2.0));
        for (int i = 0; i < 11; i++) {
            // balls.at(i).transform = glm::translate(balls.at(i).transform, glm::vec3(-0.75 + (i * 0.1), 0.0, -2.0));
            balls.at(i).Position = glm::vec2(20.0f*i - 100.0f, 10.0f*i - 50.0f);
        }
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
            ball.computeTransform(table.transform, TABLE_DIM, COORD_RES);
            ball.draw(shader);
        }
    }

};


#endif /* BILLIARD_H */