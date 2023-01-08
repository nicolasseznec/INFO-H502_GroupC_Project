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

	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
		if (InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w)))
        	inputHandler->mouse_callback(w,x,y);
	});

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

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

	RoomScene room(skybox);
	
	inputHandler.poolGame = &(room.poolGame);

    Camera camera(glm::vec3(-2.0f, 1.5f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -30.0f, -30.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	inputHandler.camera = &camera;

	glm::vec3 light_pos = glm::vec3(0.0, 1.0, -2.0);
	
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

		room.update(deltaTime);
		
		glfwPollEvents();

		// ------------- Render -------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE); 

		room.drawRoom(simpleShader, windowShader, perspective, view, camera.Position);

		// Sky
		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		skybox.bindTexture();
		skybox.draw();
		glDepthFunc(GL_LESS);
		
		room.drawMirroredRoom(camera, simpleShader, windowShader, mirrorShader);

		glfwSwapBuffers(window);
	}
	
	/*-----------------------------------------------------------*/

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}