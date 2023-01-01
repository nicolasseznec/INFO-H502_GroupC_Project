#ifndef BALL_H
#define BALL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>


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

    void draw(Shader& shader) {
        // TODO : Update the Mat4 transform with the new position/rotation
        
        Entity::draw(shader);
    }

};



#endif /* BALL_H */