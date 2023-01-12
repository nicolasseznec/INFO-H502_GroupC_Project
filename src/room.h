#ifndef ROOM_H
#define ROOM_H

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "entity.h"
#include "skybox.h"
#include "window.h"
#include "mirror.h"
#include "billiard.h"

class RoomScene
{
public:
    // Meshes
    Mesh room_mesh = Mesh(PATH_TO_OBJECTS "/room/room.obj", true);
    Mesh carpet_mesh = Mesh(PATH_TO_OBJECTS "/room/carpet.obj", true);
    Mesh bench_mesh = Mesh(PATH_TO_OBJECTS "/room/bench.obj");
    Mesh shelf_mesh = Mesh(PATH_TO_OBJECTS "/room/shelf.obj");
    Mesh mirror_frame_mesh = Mesh(PATH_TO_OBJECTS "/room/mirror_frame.obj");
    Mesh window_mesh = Mesh(PATH_TO_OBJECTS "/room/windows.obj");
    Mesh mirror_mesh = Mesh(PATH_TO_OBJECTS "/room/mirror_plane.obj");

    // Pool table
    PoolGame poolGame = PoolGame(
		PATH_TO_OBJECTS "/pool_table.obj",
		PATH_TO_TEXTURE "/pool_table/colorMap.png",
		PATH_TO_OBJECTS "/pool_ball.obj",
		PATH_TO_TEXTURE "/pool_balls/"
	);

    // Mirror
    Mirror mirror;

    // Window
    RoomWindow window;

    // Generic models
    std::vector<Entity> objects; 

    glm::mat4 transform = glm::mat4(1.0);

    RoomScene(Skybox& skybox) : 
        window(window_mesh, Texture(PATH_TO_TEXTURE "/room/window.jpg"), &skybox),
        mirror(mirror_mesh, Texture(PATH_TO_TEXTURE "/room/mirror.JPG"))
    {
        objects.push_back(Entity(room_mesh, Texture(PATH_TO_TEXTURE "/room/room_colormap.jpg"), Texture(PATH_TO_TEXTURE "/room/room_normalmap.jpg")));
        // objects.push_back(Entity(room_mesh, Texture(PATH_TO_TEXTURE "/room/room_colormap.jpg")));
        objects.push_back(Entity(carpet_mesh, Texture(PATH_TO_TEXTURE "/room/carpet_colormap.jpg"), Texture(PATH_TO_TEXTURE "/room/carpet_normalmap.jpg")));
        objects.push_back(Entity(bench_mesh, Texture(PATH_TO_TEXTURE "/room/bench_colormap.jpg")));

        Entity shelf(shelf_mesh, Texture(PATH_TO_TEXTURE "/room/Shelf.jpg"));
        shelf.transform = glm::translate(shelf.transform, glm::vec3(1.3f, 0.1f, 1.35f));
	    shelf.transform = glm::rotate(shelf.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        objects.push_back(shelf);
        
        // TODO : handle mirror frame + mirror plane in mirror.h
        Entity mirror_frame(mirror_frame_mesh, Texture(PATH_TO_TEXTURE "/room/woodplanks.jpg"));
	    mirror_frame.transform = glm::translate(mirror_frame.transform, glm::vec3(0.0f, 2.0f, -1.72f));
        objects.push_back(mirror_frame);

        // Transforms
        // for (Entity& object : objects) {
            // object.transform = this->transform * object.transform;
        // }
        // window.transform = this->transform;
        mirror.transform = this->transform * glm::translate(mirror.transform, glm::vec3(0.0f, 2.0f, -1.725f));
    }


    void update(double deltaTime) {
        poolGame.update(deltaTime);
    }

    void drawRoom(Shader& shader, Shader& windowShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position) {
        shader.use();
        setupShader(shader, perspective, view, position);

        poolGame.draw(shader);

        for (Entity& object : objects) {
            object.draw(shader);
        }
        windowShader.use();
        setupShader(windowShader, perspective, view, position);

        window.draw(windowShader);
    }

    void drawDepthMap(Shader& depthShader) {
		glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        poolGame.draw(depthShader);

        glCullFace(GL_FRONT);
        for (Entity& object : objects) {
            object.draw(depthShader);
        }
        glDisable(GL_CULL_FACE);
    }

    void drawMirroredRoom(Shader& shader, Shader& windowShader, Shader& mirrorShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position) {
        glStencilOp(GL_REPLACE, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0);

		mirrorShader.use();
		setupShader(mirrorShader, perspective, view, position);
		mirror.draw(mirrorShader);


		// 2. Render the reflected scene, but only on the mirror (where stencil==1)
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    	glStencilFunc(GL_EQUAL, 1, 0xFF);
		glColorMask(1, 1, 1, 1);

        // Draw mirrored scene
        mirror.computeMirroredProperties(perspective, view, position);
        drawRoom(shader, windowShader, mirror.mirroredPerspective, mirror.mirroredView, mirror.mirroredPosition);

        // Draw mirror texture
        glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE); 
		mirrorShader.use();
		setupShader(mirrorShader, perspective, view, position);
		mirror.draw(mirrorShader);
        glDisable(GL_STENCIL_TEST);
    }

private:
    void setupShader(Shader& shader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position) {
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", position);
    }
};

#endif /* ROOM_H */