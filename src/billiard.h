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



const glm::vec3 TABLE_DIM = glm::vec3(1.92f, 0.986f, 0.96f);
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


    // Test variables
    float timer = 0.0f;


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
            balls.at(i).Position = glm::vec2(10.0f*i - 50.0f, 20.0f*i - 100.0f);
        }
    }    

    void update(double deltaTime) {
        timer += deltaTime;
        if (timer > 5.0f) {
            timer = 0.0f;
            balls.at(2).impulse(100.0f, 90.0f);
            // std::cout << "shot : " << balls.at(2).Velocity.x << "," << balls.at(2).Velocity.y  << std::endl;
        }

        for (PoolBall& ball : balls) {
            ball.update(deltaTime);
        }

        for (int i = 0; i < balls.size(); i++) {
            for (int j = i+1; j < balls.size(); j++) {
                if (balls.at(i).checkCollision(balls.at(j))) {
                    balls.at(i).handleCollision(balls.at(j));
                }
            }
        }
        std::cout << balls.at(2).Position.y << " | " << balls.at(2).Acceleration.y << std::endl;
    }

    // TODO : separate shaders for table and balls ?
    void draw(Shader& shader) {
        table.draw(shader);
        
        for (PoolBall& ball : balls) {
            ball.computeTransform(table.transform, TABLE_DIM, COORD_RES);
            ball.draw(shader);
        }
    }
};


#endif /* BILLIARD_H */