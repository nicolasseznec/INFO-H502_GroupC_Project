#ifndef MIRROR_H
#define MIRROR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    void computeMirroredProperties(Camera& camera) {
        glm::mat4 perspective = camera.GetProjectionMatrix();
        glm::mat4 view = camera.GetViewMatrix();

        mirroredPerspective = glm::scale(perspective, glm::vec3(-1.0f, 1.0f, 1.0f));

		// TODO : automatically compute
		glm::vec3 mirroredFront = camera.Front;
		mirroredFront.z *= -1; 

		glm::vec3 mirroredUp = camera.Up;
		mirroredUp.z *= -1;

		// TODO : automatically compute
		mirroredPosition = camera.Position; 
		mirroredPosition.z = -7.4f - mirroredPosition.z;

		mirroredView = glm::lookAt(mirroredPosition, mirroredPosition + mirroredFront, mirroredUp);
    }

    void draw(Shader& shader) {
        Entity::draw(shader);
	}
};

#endif /* MIRROR_H */