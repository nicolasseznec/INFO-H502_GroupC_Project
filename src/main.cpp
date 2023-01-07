#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>
#include <vector>

#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "mesh.h"
#include "entity.h"
#include "input.h"
#include "debug.h"
#include "skybox.h"
#include "billiard.h"
#include "room.h"

const int width = 800;
const int height = 800;

int main(int argc, char* argv[])
{
	/*-----------------------------------------------------------*/
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Exercise 10", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);

	InputHandler inputHandler = InputHandler();

	// To access the input handler methods from the callbacks
	glfwSetWindowUserPointer(window, &inputHandler);

	// glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
		if (InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w)))
        	inputHandler->mouse_callback(w,x,y);
	});

	// glfwSetScrollCallback(window, scroll_callback);
	glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y) {
		if (InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w)))
        	inputHandler->scroll_callback(w,x,y);
	});

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

#ifndef NDEBUG
    setupDebug();
#endif

	/*-----------------------------------------------------------*/

	Shader shader(PATH_TO_SHADERS "/refraction.vert", 
				  PATH_TO_SHADERS "/refraction.frag");
	
	Shader cubeMapShader(PATH_TO_SHADERS "/cubeMap.vert", 
						 PATH_TO_SHADERS "/cubeMap.frag");

	Shader simpleShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/simple.frag");

	Shader windowShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/window.frag");
						
	Shader mirrorShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/mirror.frag");

	PoolGame poolGame(
		PATH_TO_OBJECTS "/pool_table.obj",
		PATH_TO_TEXTURE "/pool_table/colorMap.png",
		PATH_TO_OBJECTS "/pool_ball.obj",
		PATH_TO_TEXTURE "/pool_balls/"
	);
	inputHandler.poolGame = &poolGame;
	
	RoomScene room;

	Mesh window_mesh(PATH_TO_OBJECTS "/room/windows.obj");
	Entity window_obj(window_mesh, Texture(PATH_TO_TEXTURE "/room/window.jpg"));
	window_obj.transform = glm::translate(window_obj.transform, glm::vec3(0.0f, -1.0f, -2.0f));

	Mesh mirror_mesh(PATH_TO_OBJECTS "/room/mirror_plane.obj");
	Entity mirror(mirror_mesh, Texture(PATH_TO_TEXTURE "/room/window.jpg"));
	mirror.transform = glm::translate(mirror.transform, glm::vec3(0.0f, 1.0f, -3.7f));

	Mesh mirror_frame_mesh(PATH_TO_OBJECTS "/room/mirror_frame.obj");
	Entity mirror_frame(mirror_frame_mesh, Texture(PATH_TO_TEXTURE "/room/woodplanks.jpg"));
	mirror_frame.transform = glm::translate(mirror_frame.transform, glm::vec3(0.0f, 1.0f, -3.7f));

	Mesh shelf_mesh(PATH_TO_OBJECTS "/room/shelf.obj");
	Entity shelf(shelf_mesh, Texture(PATH_TO_TEXTURE "/room/Shelf.jpg"));
	shelf.transform = glm::translate(shelf.transform, glm::vec3(1.3f, -0.9f, -0.65f));
	shelf.transform = glm::rotate(shelf.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// mirror.transform = glm::scale(mirror.transform, glm::vec3(0.25f, 0.25f, 1.0f));
	// mirror.transform = glm::rotate(mirror.transform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	

	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/yokohama3/";
	std::map<std::string, GLenum> facesToLoad = {
		{ "posx.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{ "posy.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{ "posz.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{ "negx.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{ "negy.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{ "negz.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	Skybox skybox(pathToCubeMap, facesToLoad , pathCube);


    Camera camera(glm::vec3(-2.0f, 1.5f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -30.0f, -30.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	inputHandler.camera = &camera;

	glm::vec3 light_pos = glm::vec3(0.0, 1.0, -2.0);
	
	// model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	/*
	pool_table.transform = glm::translate(pool_table.transform, glm::vec3(0.0, -1.0, -2.0));
	ball.transform = glm::translate(ball.transform, glm::vec3(0.0, 0.0, -2.0));
	ball2.transform = glm::translate(ball2.transform, glm::vec3(-0.5, 0.0, -2.25));
	ball2.transform = glm::rotate(ball2.transform, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	*/

	//Rendering
	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;

	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);

	simpleShader.use();
	simpleShader.setFloat("shininess", 32.0f);
	simpleShader.setVector3f("materialColour", materialColour);
	simpleShader.setVector3f("light.light_pos", light_pos);
	simpleShader.setFloat("light.ambient_strength", ambient);
	simpleShader.setFloat("light.diffuse_strength", diffuse);
	simpleShader.setFloat("light.specular_strength", specular);
	simpleShader.setFloat("light.constant", 1.0);
	simpleShader.setFloat("light.linear", 0.14);
	simpleShader.setFloat("light.quadratic", 0.07);

	// shader.use();
	// shader.setFloat("refractionIndice", 1.52);
	windowShader.use();
	windowShader.setFloat("refractionIndice", 1.0);

	/*-----------------------------------------------------------*/

	// TODO : put fps/time management in a general update system (used to update the objects in the scene, etc...)

	double prev = 0;
	int frames = 0;
	
	//fps function
	auto fps = [&](double now) {
		double delta = now - prev;
		frames++;
		if (delta > 0.5) {
			prev = now;
			const double fpsCount = (double)frames / delta;
			frames = 0;
			// std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};
	
	double prevTime = 0;
	double deltaTime = 0;

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {

		double now = glfwGetTime();
		deltaTime = now - prevTime;
		prevTime = now;

		fps(now);

		inputHandler.processInput(window, deltaTime);
		view = camera.GetViewMatrix();
		perspective = camera.GetProjectionMatrix();
		
		poolGame.update(deltaTime);
		
		glfwPollEvents();

		// ------------- Render -------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// ----------------Test : Mirror------------------

		/*
		// 1. render the scene from a mirrored camera
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE); 
		glCullFace(GL_FRONT); // Actually culls the mirrored back face somehow

		glm::mat4 mirroredPerspective = glm::scale(perspective, glm::vec3(-1.0f, 1.0f, 1.0f));
		// glm::mat4 mirroredPerspective = perspective;

		// TODO : automatically compute
		glm::vec3 mirroredFront = camera.Front;
		// mirroredFront = glm::reflect(mirroredFront, glm::vec3(1.0f, 0.0f, 0.0f));
		mirroredFront.z *= -1; 

		glm::vec3 mirroredUp = camera.Up;
		mirroredUp.z *= -1;

		// TODO : automatically compute
		glm::vec3 mirroredPosition = camera.Position; 
		// mirroredPosition.z = -3.0f - glm::abs(mirroredPosition.z + 3.0f);
		mirroredPosition.z = -7.0f - mirroredPosition.z;

		glm::mat4 mirroredView = glm::lookAt(mirroredPosition, mirroredPosition + mirroredFront, mirroredUp);
		// glm::mat4 mirroredView = view;

		
		simpleShader.use();
		simpleShader.setMatrix4("V", mirroredView); //TODO
		simpleShader.setMatrix4("P", mirroredPerspective);
		simpleShader.setVector3f("u_view_pos", mirroredPosition); //TODO

		poolGame.draw(simpleShader);
		room.draw(simpleShader);
		
		windowShader.use();
		windowShader.setMatrix4("V", mirroredView); //TODO
		windowShader.setMatrix4("P", mirroredPerspective); 
		windowShader.setVector3f("u_view_pos", mirroredPosition); //TODO
		windowShader.setInteger("cubemapSampler", 1);
		skybox.bindTexture(1);
		window_obj.draw(windowShader);
		
		// 2. Render mirror and set stencil buffer to 1 on touched pixels
		glClear(GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0); // TODO : see if necessary

		mirrorShader.use();
		mirrorShader.setMatrix4("V", view);
		mirrorShader.setMatrix4("P", perspective);
		mirrorShader.setVector3f("u_view_pos", camera.Position);
		mirror.draw(mirrorShader);

		// 3. Render the normal scene, but not on the mirror (where stencil==1)
		glDisable(GL_CULL_FACE);
		// glCullFace(GL_BACK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glColorMask(1, 1, 1, 1);
		// -----------------------------------------------------------------
		*/

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE); 

		simpleShader.use();
		simpleShader.setMatrix4("V", view);
		simpleShader.setMatrix4("P", perspective);
		simpleShader.setVector3f("u_view_pos", camera.Position);

		poolGame.draw(simpleShader);
		room.draw(simpleShader);
		mirror_frame.draw(simpleShader);
		shelf.draw(simpleShader);
		
		windowShader.use();
		windowShader.setMatrix4("V", view);
		windowShader.setMatrix4("P", perspective);
		windowShader.setVector3f("u_view_pos", camera.Position);
		windowShader.setInteger("cubemapSampler", 1);
		skybox.bindTexture(1);
		window_obj.draw(windowShader);
		
		// window_obj.draw(simpleShader);

		// Sky
		/*
		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		skybox.bindTexture();
		skybox.draw();
		glDepthFunc(GL_LESS);
		*/

		//----------------------------------------------------
		// 1. Render mirror and set stencil buffer to 1 on touched pixels
		
		glStencilOp(GL_REPLACE, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0); // TODO : see if necessary

		mirrorShader.use();
		mirrorShader.setMatrix4("V", view);
		mirrorShader.setMatrix4("P", perspective);
		mirrorShader.setVector3f("u_view_pos", camera.Position);
		mirror.draw(mirrorShader);

		// 2. Render the reflected scene, but only on the mirror (where stencil==1)
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    	glStencilFunc(GL_EQUAL, 1, 0xFF);
    	// glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glColorMask(1, 1, 1, 1);

		glm::mat4 mirroredPerspective = glm::scale(perspective, glm::vec3(-1.0f, 1.0f, 1.0f));
		// glm::mat4 mirroredPerspective = perspective;

		// TODO : automatically compute
		glm::vec3 mirroredFront = camera.Front;
		// mirroredFront = glm::reflect(mirroredFront, glm::vec3(1.0f, 0.0f, 0.0f));
		mirroredFront.z *= -1; 

		glm::vec3 mirroredUp = camera.Up;
		mirroredUp.z *= -1;

		// TODO : automatically compute
		glm::vec3 mirroredPosition = camera.Position; 
		mirroredPosition.z = -7.4f - mirroredPosition.z;

		glm::mat4 mirroredView = glm::lookAt(mirroredPosition, mirroredPosition + mirroredFront, mirroredUp);
		// glm::mat4 mirroredView = view;

		
		simpleShader.use();
		simpleShader.setMatrix4("V", mirroredView); //TODO
		simpleShader.setMatrix4("P", mirroredPerspective);
		simpleShader.setVector3f("u_view_pos", mirroredPosition); //TODO

		poolGame.draw(simpleShader);
		room.draw(simpleShader);
		mirror_frame.draw(simpleShader);
		shelf.draw(simpleShader);

		windowShader.use();
		windowShader.setMatrix4("V", mirroredView); //TODO
		windowShader.setMatrix4("P", mirroredPerspective); 
		windowShader.setVector3f("u_view_pos", mirroredPosition); //TODO
		windowShader.setInteger("cubemapSampler", 1);
		skybox.bindTexture(1);
		window_obj.draw(windowShader);
		

		glfwSwapBuffers(window);
	}
	
	/*-----------------------------------------------------------*/

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}