#ifndef BALL_H
#define BALL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "texture.h"
#include "mesh.h"
#include "entity.h"


const float MASS = 1.0f;
const float RADIUS = 2.7f;
const float FRICTION = 1.0f;
const float RESTITUTION = 0.9f;
const float STOP_TH = 0.5f;

class PoolBall : public Entity
{
public: 
    glm::vec2 Position = glm::vec2(0.0f);
    glm::vec2 PreviousPos = glm::vec2(0.0f);
    glm::vec2 Velocity = glm::vec2(0.0f);
    glm::vec2 Acceleration = glm::vec2(0.0f);

    float Radius;
    float Mass;

    int sens = 1;

    bool firstCompute = true;
    glm::mat4 Rotation = glm::mat4(1.0f);
    glm::vec3 relativeDir = glm::vec3(0.0f);

    PoolBall(Mesh& model, Texture texture, float Radius=RADIUS, float Mass=MASS) : Entity(model, texture), Radius(Radius), Mass(Mass) {

    }

    void update(float deltaTime) {
        Velocity += Acceleration * deltaTime; // delta time squared ?
        checkStopThreshold();
        Acceleration =  Velocity * -FRICTION/Mass;  // friction
        Position += Velocity * deltaTime;
    }

    bool checkCollision(PoolBall& other) {
        float minDist = (Radius + other.Radius);
        return glm::length2(Position - other.Position) <= (minDist * minDist);
    }

    void handleCollision(PoolBall& other) {
        glm::vec2 deltaPos = Position - other.Position;
        glm::vec2 deltaPosN = glm::normalize(deltaPos);
        glm::vec2 deltaVel = Velocity - other.Velocity;

        float invM1 = 1/Mass;
        float invM2 = 1/other.Mass;
        float sumInvM = invM1 + invM2;   // M1.M2/(M1+M2)

        // Correct Overlapping
        float dist = glm::length(deltaPos);
        glm::vec2 correction = deltaPosN * (Radius + other.Radius - dist);
        Position += correction * invM1/sumInvM;
        other.Position -= correction * invM2/sumInvM;

        // Compute impulse
        float vn = glm::dot(deltaVel, deltaPosN);
        if (vn > 0.0f) return;
        glm::vec2 impulse = deltaPosN * -(1.0f + RESTITUTION) * vn/sumInvM;

        // Update velocities
        Velocity += impulse * invM1;
        other.Velocity -= impulse * invM2;
    }

    void checkBounds(float maxX, float maxY) {
        if (Position.x + Radius > maxX) {
            // EAST RAIL
            Position.x = maxX - Radius;
            if (Velocity.x > 0.0f) Velocity.x *= -1.0f;
        }
        else if (Position.x - Radius < -maxX) {
            // WEST RAIL
            Position.x = Radius - maxX;
            if (Velocity.x < 0.0f) Velocity.x *= -1.0f;
        }

        if (Position.y + Radius > maxY) {
            // NORTH RAIL
            Position.y = maxY - Radius;
            if (Velocity.y > 0.0f) Velocity.y *= -1.0f;
        }
        else if (Position.y - Radius < -maxY) {
            // SOUTH RAIL
            Position.y = Radius - maxY;
            if (Velocity.y < 0.0f) Velocity.y *= -1.0f;
        }
    }

    void computeTransform(glm::mat4 table_transform, glm::vec3 table_dim, glm::vec3 coord_res) {
        glm::vec3 res = table_dim/coord_res;

        if (firstCompute) {
            firstCompute = false;
            PreviousPos = Position;
        }
        glm::vec2 deltaPos = Position - PreviousPos;
        float angleRad = glm::length(deltaPos) / Radius;

        relativeDir = glm::vec3(deltaPos.y, 0.0f, deltaPos.x) * res;
        if (glm::length(deltaPos) == 0.0f) relativeDir = glm::vec3(0.0f, 0.0f, 1.0f);

        // There is far from efficient but it seems to work
        // Rotate the ball
        glm::mat4 newRotation = glm::rotate(glm::mat4(1.0f), angleRad, glm::cross(relativeDir, glm::vec3(0.0f, -1.0f, 0.0f)));
        Rotation = newRotation * Rotation;

        // Place at the right position
        glm::mat4 relativePos =  glm::translate(glm::mat4(1.0f), glm::vec3(Position.y, 1.0f, Position.x) * res);

        this->transform = table_transform * relativePos * Rotation;
        PreviousPos = Position;
    }

    void impulse(float magnitude, float angle) {
        glm::vec2 force = glm::rotate(glm::vec2(1.0f, 0.0f), glm::radians(angle)) * magnitude;
        Velocity += force;
    }

private:
    void checkStopThreshold() {
        if (glm::abs(Velocity.x) < STOP_TH) Velocity.x = 0.0f;
        if (glm::abs(Velocity.y) < STOP_TH) Velocity.y = 0.0f;
    }
};

#endif /* BALL_H */