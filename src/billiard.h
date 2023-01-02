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

// TEST
#include <cstdlib>


#include "texture.h"
#include "mesh.h"
#include "entity.h"
#include "ball.h"



const glm::vec3 TABLE_DIM = glm::vec3(1.92f, 0.986f, 0.96f);
const glm::vec3 COORD_RES = glm::vec3(200.0f, 1.0f, 100.0f);

struct PoolPocket {
    // position
    // radius
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
        
        for (int i = 0; i < 16; i++) {
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << i;
            Texture texture = Texture((ballTexturePath + "ball_" + ss.str() + ".jpg").c_str()); // TODO : not hardcoding the balls name ?
            balls.push_back(PoolBall(ballMesh, texture));
            balls.at(i).Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        table.transform = glm::translate(table.transform, glm::vec3(0.0, -1.0, -2.0));
        setupBalls();
    }

    void update(double deltaTime) {
        timer += deltaTime;
        if (timer > 5.0f) {
            timer = 0.0f;

            balls.at(0).impulse(200.0f, std::rand() % 360);
            // balls.at(2).impulse(100.0f, 135.0f);
        }

        for (PoolBall& ball : balls) {
            ball.update(deltaTime);
        }

        
        for (int i = 0; i < balls.size(); i++) {
            PoolBall& ball = balls.at(i);

            for (int j = i+1; j < balls.size(); j++) {
                if (ball.checkCollision(balls.at(j))) {
                    ball.handleCollision(balls.at(j));
                }
            }

            ball.checkBounds(COORD_RES.z * 0.5f, COORD_RES.x * 0.5f);
            ball.computeTransform(table.transform, TABLE_DIM, COORD_RES);
        }
    }

    // TODO : separate shaders for table and balls ?
    void draw(Shader& shader) {
        table.draw(shader);
        
        for (PoolBall& ball : balls) {
            ball.draw(shader);
        }
    }

private: 
    void setupBalls() {
        if (balls.size() != 16) return;

        // Place balls in triangle
        const float maxX = COORD_RES.x * 0.5f;
        const float r = RADIUS + 0.1f;
        const float h = glm::sqrt(3) * r;
        int indexes[15]  = {9, 7, 12, 15, 8, 1, 6, 10, 3, 14, 11, 2, 13, 4, 5};
        int length = 1;
        int number = 1;
        
        balls[0].Position = glm::vec2(0.0f, maxX * 0.5f); 
        glm::vec2 current = glm::vec2(0.0f, -maxX * 0.5f);

        for (int i=0; i<15; i++) {
            int index = indexes[i];

            if (number < length) {
                balls[index].Position = current;
                current.x = current.x + r*2;
                number++;
            }
            else {
                balls[index].Position = current;
                current.y = current.y - h;
                current.x = current.x - r*(2*length-1);

                length++;
                number = 1;
            }
        }
    }
};


#endif /* BILLIARD_H */