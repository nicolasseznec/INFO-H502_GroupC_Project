#ifndef CUE_H
#define CUE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "texture.h"
#include "mesh.h"
#include "entity.h"
#include "ball.h"

const float COOLDOWN = 5.0f;
const float HIT_DURATION = 0.1f;

const float ROTATE_SPEED = 50.0f;
const float DISTANCE_SPEED = 0.3f;
const float DISTANCE_MIN = 0.05f;
const float DISTANCE_MAX = 0.3f;


class PoolCue : public Entity
{
public: 
    glm::vec2 Position = glm::vec3(0.0f);
    float azimuthal = -90.0f;
    float latitude = 15.0f;
    float distance = DISTANCE_MIN;
    float force = 300.0f;

    bool enabled = false;
    bool takeInput = true;
    float shootTimer = 0.0f;
    float cooldownTimer = 0.0f;
    float shotDistance = 0.0f;

    glm::mat4 disabledTransform;

    PoolCue(Mesh& model, Texture texture) : Entity(model, texture) {
        disabledTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.62f, 1.27f, 0.565f));
        disabledTransform = glm::rotate(disabledTransform, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        disabledTransform = glm::rotate(disabledTransform, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Return true if shot this frame
    bool update(float deltaTime, glm::vec3 cueBallPos) {
        if (!enabled) return false;

        if (shootTimer > 0.0f) {
            shootTimer -= deltaTime;
            distance = shotDistance * shootTimer/HIT_DURATION;

            if (shootTimer <= 0.0f) {
                cooldownTimer = COOLDOWN;
                return true;
            }
        }
        else if (cooldownTimer > 0.0f) {
            cooldownTimer -= deltaTime;
            
            float delta = COOLDOWN - cooldownTimer;
            if (delta <= 1.0f) {
                distance = delta * shotDistance;
            }

            if (cooldownTimer <= 0.0f) {
                takeInput = true;
                Position = glm::vec2(cueBallPos);
                distance = shotDistance;
            }
        }
        else {
            Position = glm::vec2(cueBallPos);
        }

        return false;
    }

    void computeTransform(glm::mat4 table_transform, glm::vec3 table_dim, glm::vec3 coord_res) {
        if (!enabled) {
            this->transform = table_transform * disabledTransform;
            return;
        }

        glm::vec3 res = table_dim/coord_res;

        glm::mat4 relativePos =  glm::translate(glm::mat4(1.0f), glm::vec3(Position.y, coord_res.y, Position.x) * res);
        relativePos = glm::rotate(relativePos, glm::radians(azimuthal + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        relativePos = glm::rotate(relativePos, glm::radians(latitude), glm::vec3(0.0f, 0.0f, 1.0f));
        relativePos =  glm::translate(relativePos, glm::vec3(distance, 0.0f, 0.0f));

        this->transform = table_transform * relativePos;
    }

    void turn(int direction, float deltaTime) {
        if (!enabled || !takeInput) return;

        int dir = direction >= 0 ? 1 : -1;
        azimuthal += dir * deltaTime * ROTATE_SPEED;
    }

    void changeDistance(int direction, float deltaTime) {
        if (!enabled || !takeInput) return;

        int dir = direction >= 0 ? 1 : -1;
        distance += dir * deltaTime * DISTANCE_SPEED;
        limitDistance();
    }

    void shoot() {
        if (!enabled || !takeInput) return;

        // force range : 50 -> 300
        force = 50.0f + 250.0f * (distance - DISTANCE_MIN)/(DISTANCE_MAX - DISTANCE_MIN);

        shootTimer = HIT_DURATION;
        shotDistance = distance;
        takeInput = false;
    }

    void switchEnable() {
        enabled = !enabled;
    }

private:
    void limitDistance() {
        if (distance > DISTANCE_MAX) distance = DISTANCE_MAX;
        if (distance < DISTANCE_MIN) distance = DISTANCE_MIN;
    }

};

#endif /* CUE_H */