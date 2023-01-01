#ifndef BALL_H
#define BALL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "texture.h"
#include "mesh.h"
#include "entity.h"


const float MASS = 1.0f;
const float RADIUS = 1.0f;
// friction


class PoolBall : public Entity
{
public: 
    glm::vec2 Position = glm::vec2(0.0, 0.0);
    glm::vec2 Velocity = glm::vec2(0.0, 0.0);
    glm::vec2 Acceleration = glm::vec2(0.0, 0.0);

    float Radius;
    float Mass;

    PoolBall(Mesh& model, Texture texture, float Radius=RADIUS, float Mass=MASS) : Entity(model, texture), Radius(Radius), Mass(Mass) {

    }

    void update(double deltaTime) {
        // Increment position with velocity and acceleration
    }

    bool checkCollision(PoolBall other) {
        // TODO : returns true if colliding with other
        return false;
    }

    /*
    bool checkCollision(Wall wall) {
        // TODO : returns true if colliding with wall
    }
    */

    void handleCollision(PoolBall other) {
        // Separate balls and recompute velocity
    }

    /*
    void handleCollision(Wall wall) {
        // Move ball and recompute velocity
    }
    */

    void computeTransform(glm::mat4 table_transform, glm::vec3 table_dim, glm::vec3 coord_res) {
        // TODO : Update the Mat4 transform with the new position/rotation
        glm::mat4 relPos =  glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, 1.0f, Position.y) * table_dim/coord_res);

        this->transform = table_transform * relPos;
    }
};



#endif /* BALL_H */