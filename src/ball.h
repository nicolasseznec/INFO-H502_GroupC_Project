#ifndef BALL_H
#define BALL_H

#include <vector>

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


const float POCKET_RADIUS = 4.8f;
const float POCKET_DEPTH = 8.0f;
const float POCKET_MINDIST = 10.0f;
const float POCKET_X = 56.0f;
const float POCKET_X2 = 52.0f;
const float POCKET_Y = 102.0f;


struct PoolPocket {
    glm::vec3 Position;
    float Radius;
    float depth;
    glm::vec3 Direction;
    float minDist;

    PoolPocket() {}

    PoolPocket(float x, float y, float angle, float Radius = POCKET_RADIUS, float depth = POCKET_DEPTH, float minDist = POCKET_MINDIST) 
    : minDist(minDist), Radius(Radius), depth(depth) 
    {
        Position = glm::vec3(x, y, 0.0f);
        setDirection(angle);
    }

    void setDirection(float angle) {
        Direction = glm::rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

class PoolBall : public Entity
{
public: 
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 PreviousPos = glm::vec3(0.0f);
    glm::vec3 Velocity = glm::vec3(0.0f);
    glm::vec3 Acceleration = glm::vec3(0.0f);

    float Radius;
    float Mass;

    bool enteredPocket = false;
    PoolPocket Pocket;

    bool firstCompute = true;
    glm::mat4 Rotation = glm::mat4(1.0f);
    glm::vec3 relativeDir = glm::vec3(0.0f);

    PoolBall(Mesh& model, Texture texture, float Radius=RADIUS, float Mass=MASS) : Entity(model, texture), Radius(Radius), Mass(Mass) {

    }

    void update(float deltaTime) {
        Velocity += Acceleration * deltaTime;
        checkStopThreshold();
        Acceleration =  Velocity * -FRICTION/Mass;  // friction
        Position += Velocity * deltaTime;

        if (!enteredPocket) {
            Position.z = 0.0f;
            Velocity.z = 0.0f;
            Acceleration.z = 0.0f;
        }
    }

    bool checkCollision(PoolBall& other) {
        if (enteredPocket || other.enteredPocket) return false;

        float minDist = (Radius + other.Radius);
        return glm::length2(Position - other.Position) <= (minDist * minDist);
    }

    void handleCollision(PoolBall& other) {
        glm::vec3 deltaPos = Position - other.Position;
        glm::vec3 deltaPosN = glm::normalize(deltaPos);
        glm::vec3 deltaVel = Velocity - other.Velocity;

        float invM1 = 1/Mass;
        float invM2 = 1/other.Mass;
        float sumInvM = invM1 + invM2;   // M1.M2/(M1+M2)

        // Correct Overlapping
        float dist = glm::length(deltaPos);
        glm::vec3 correction = deltaPosN * (Radius + other.Radius - dist);
        Position += correction * invM1/sumInvM;
        other.Position -= correction * invM2/sumInvM;

        // Compute impulse
        float vn = glm::dot(deltaVel, deltaPosN);
        if (vn > 0.0f) return;
        glm::vec3 impulse = deltaPosN * -(1.0f + RESTITUTION) * vn/sumInvM;

        // Update velocities
        Velocity += impulse * invM1;
        other.Velocity -= impulse * invM2;
    }

    void checkTable(std::vector<PoolPocket>& pockets, float maxX, float maxY) {
        if (enteredPocket) {
            updateInPocket();
            return;
        }

        if (insideBounds(maxX, maxY)) return;

        bool inPocket = false;
        for (PoolPocket pocket : pockets) {
            if (checkPocket(pocket)) {
                inPocket = true;
                break;
            }
        }

        if (!inPocket) checkBounds(maxX, maxY);
    }

    bool checkPocket(PoolPocket pocket) {
        glm::vec2 deltaPos(Position - pocket.Position);
        float distance2 = glm::length2(deltaPos);

        float minRadius = pocket.Radius - Radius;
        float minRadius2 = minRadius*minRadius;

        if (distance2 <= minRadius2) {
            // Ball is inside the hole (throat)
            enteredPocket = true;
            this->Pocket = pocket;
            return true;
        }
        else if (distance2 <= pocket.minDist * pocket.minDist) {
            // Ball is in the mouth of the pocket
            glm::vec3 projPos = pocket.Position + pocket.Direction * glm::dot(Position - pocket.Position, pocket.Direction)/glm::length2(pocket.Direction);
            glm::vec2 delta(Position - projPos);
            
            glm::vec2 normal = glm::normalize(-delta);
            glm::vec2 flatVelocity(Velocity);
            

            if (glm::length2(delta) > minRadius2 && glm::dot(normal, flatVelocity) < 0.0f) {
                // ball is colliding with borders of the mouth
            
                // Correct position
                float distance = glm::length(delta);
                Position = projPos + glm::vec3(delta, Position.z) * (minRadius/distance);

                // Bouncing
                
                if (glm::dot(normal, flatVelocity) < 0.0f) {
                    Velocity = glm::vec3(glm::reflect(flatVelocity, normal), Velocity.z);
                }
            }
            return true;
        }

        return false;
    }

    bool insideBounds(float maxX, float maxY) {
        return (glm::abs(Position.x) <= maxX-Radius) && (glm::abs(Position.y) <= maxY-Radius);
    }

    void updateInPocket() {
        glm::vec2 deltaPos(Position - Pocket.Position);
        float distance2 = glm::length2(deltaPos);

        float minRadius = Pocket.Radius - Radius;

        if(distance2 > minRadius*minRadius) {
            // Ball is colliding with the borders of the hole

            float distance = glm::length(deltaPos);
            
            // Correct position
            Position = Pocket.Position + glm::vec3(deltaPos, Position.z) * (minRadius/distance);
            // Bouncing
            glm::vec2 normal = glm::normalize(-deltaPos);
            
            glm::vec2 flatVelocity(Velocity);
            if (glm::dot(normal, flatVelocity) < 0.0f) {
                Velocity = glm::vec3(glm::reflect(flatVelocity, normal) * 0.7f, Velocity.z);
            }
        }
        // falling in the hole
        Acceleration.z = -200.0f;
        if (Position.z < -Pocket.depth) {
            Position.z = -Pocket.depth;
            if (Velocity.z < 0.0f) Velocity.z *= -1 * 0.8f;
        }
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

        // This is far from efficient but it seems to work
        // Rotate the ball
        glm::mat4 newRotation = glm::rotate(glm::mat4(1.0f), angleRad, glm::cross(relativeDir, glm::vec3(0.0f, -1.0f, 0.0f)));
        Rotation = newRotation * Rotation;

        // Place at the right position
        // glm::mat4 relativePos =  glm::translate(glm::mat4(1.0f), glm::vec3(Position.y, 1.0f, Position.x) * res);
        glm::mat4 relativePos =  glm::translate(glm::mat4(1.0f), glm::vec3(Position.y, Position.z + coord_res.y, Position.x) * res);

        this->transform = table_transform * relativePos * Rotation;
        PreviousPos = Position;
    }

    void impulse(float magnitude, float angle) {
        glm::vec2 force = glm::rotate(glm::vec2(1.0f, 0.0f), glm::radians(angle)) * magnitude;
        Velocity += glm::vec3(force, 0.0f);
    }

    void reset(float x = 0.0f, float y = 0.0f) {
        Position = glm::vec3(x, y, 0.0f); 
        Velocity = glm::vec3(0.0f);
        Acceleration = glm::vec3(0.0f);
        enteredPocket = false;
        Rotation = glm::mat4(1.0f);
    }

private:
    void checkStopThreshold() {
        if (glm::abs(Velocity.x) < STOP_TH) Velocity.x = 0.0f;
        if (glm::abs(Velocity.y) < STOP_TH) Velocity.y = 0.0f;
    }
};

#endif /* BALL_H */