#ifndef MIRROR_H
#define MIRROR_H

// Some parts of the code were taken from https://learnopengl.com/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "entity.h"
#include "camera.h"

class Mirror : public Entity
{
public:
    glm::mat4 mirroredPerspective;
    glm::mat4 mirroredView;
    glm::vec3 mirroredPosition;

    Mirror(Mesh& model, Texture texture) : Entity(model, texture) {
    }

    void computeMirroredProperties(glm::mat4 perspective, glm::mat4 view, glm::vec3 position) {
        mirroredPerspective = perspective;

        glm::vec3 point = glm::vec3(transform * glm::vec4(glm::vec3(0.0f), 1.0f));
        glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f); // TODO : take into account rotations

        glm::vec4 mirrorPlane = planeFromPointNormal(point, normal);
        glm::mat4 reflection = matrixReflect(mirrorPlane);
        mirroredView = view * reflection;
        mirroredPosition = reflection * glm::vec4(position, 1.0f);
    }

    void draw(Shader& shader) {
        Entity::draw(shader);
	}

private:
    // Helper functions taken from :
    // https://gamedev.stackexchange.com/questions/198098/how-do-i-calculate-a-matrix-to-render-a-mirror-about-a-plane-using-glm
    glm::vec4 planeFromPointNormal(glm::vec3 pt, glm::vec3 normal) {
        auto norm = glm::normalize(normal);
        return { norm.x, norm.y, norm.z, -glm::dot(pt, norm) };
    }

    glm::mat4 matrixReflect(glm::vec4 plane) {
        return glm::mat4{
            -2 * plane.x * plane.x + 1,  -2 * plane.y * plane.x,      -2 * plane.z * plane.x,        0,
            -2 * plane.x * plane.y,      -2 * plane.y * plane.y + 1,  -2 * plane.z * plane.y,        0,
            -2 * plane.x * plane.z,      -2 * plane.y * plane.z,      -2 * plane.z * plane.z + 1,    0,
            -2 * plane.x * plane.w,      -2 * plane.y * plane.w,      -2 * plane.z * plane.w,        1
        };
    }
};

#endif /* MIRROR_H */