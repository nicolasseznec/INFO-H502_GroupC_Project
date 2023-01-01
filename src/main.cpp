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

#include "camera.h"
#include "shader.h"
// #include "object.h"
#include "entity.h"
#include "mesh.h"
#include "input.h"
#include "debug.h"
#include "skybox.h"

const int width = 800;
const int height = 800;

GLuint loadTexture(const char* file);


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

	GLuint texture = loadTexture(PATH_TO_TEXTURE "/pool_table/colorMap.png");
	GLuint textureBall = loadTexture(PATH_TO_TEXTURE "/pool_balls/ball_14.jpg");

	/*
	Object pool_table(PATH_TO_OBJECTS "/pool_table.obj");
	pool_table.makeObject(simpleShader);

	Object ball(PATH_TO_OBJECTS "/pool_ball.obj");
	ball.makeObject(simpleShader);
	*/

	Mesh table_mesh(PATH_TO_OBJECTS "/pool_table.obj");
	Mesh ball_mesh(PATH_TO_OBJECTS "/pool_ball.obj");

	Entity pool_table(table_mesh, texture);
	Entity ball(ball_mesh, textureBall);


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
	// TODO : Skybox use mesh
	Skybox skybox(pathToCubeMap, facesToLoad , pathCube, cubeMapShader);


    Camera camera(glm::vec3(0.0, 0.0, 0.1));
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	inputHandler.camera = &camera;

	// glm::vec3 light_pos = glm::vec3(1.0, 2.0, 1.5);
	glm::vec3 light_pos = glm::vec3(0.0, 5.0, -2.0);
	
	/*
	pool_table.model = glm::translate(pool_table.model, glm::vec3(0.0, -1.0, -2.0));
	ball.model = glm::translate(ball.model, glm::vec3(0.0, 0.0, -2.0));
	// model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	*/
	pool_table.transform = glm::translate(pool_table.transform, glm::vec3(0.0, -1.0, -2.0));
	ball.transform = glm::translate(ball.transform, glm::vec3(0.0, 0.0, -2.0));


	//Rendering
	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;

	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);

	simpleShader.use();
	simpleShader.setFloat("shininess", 32.0f);
	simpleShader.setVector3f("materialColour", materialColour);
	simpleShader.setFloat("light.ambient_strength", ambient);
	simpleShader.setFloat("light.diffuse_strength", diffuse);
	simpleShader.setFloat("light.specular_strength", specular);
	simpleShader.setFloat("light.constant", 1.0);
	simpleShader.setFloat("light.linear", 0.14);
	simpleShader.setFloat("light.quadratic", 0.07);

	shader.use();
	shader.setFloat("refractionIndice", 1.52);

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
			std::cout << "\r FPS: " << fpsCount;
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
		glfwPollEvents();

		// ------------- Render -------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		simpleShader.use();
		simpleShader.setMatrix4("V", view);
		simpleShader.setMatrix4("P", perspective);
		simpleShader.setVector3f("u_view_pos", camera.Position);

		/*
		// Table
		simpleShader.setInteger("u_texture", 0);  // Set the texture unit to use (set with GL_TEXTURE0, GL_TEXTURE1, ...) (by default 0) (Could be done before the loop)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		simpleShader.setMatrix4("M", pool_table.model);
		simpleShader.setMatrix4("itM", glm::transpose(glm::inverse(pool_table.model)));
		pool_table.draw();

		// TODO : automate oject drawing (texture binding, uniforms setup, ...)
		// TODO : All balls use the same model (no need to load it 16 times from the file) 
		// Ball
		simpleShader.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureBall);
		
		simpleShader.setMatrix4("M", ball.model);
		simpleShader.setMatrix4("itM", glm::transpose(glm::inverse(ball.model)));
		ball.draw();
		*/

		pool_table.draw(simpleShader);
		ball.draw(simpleShader);

		// Sky
		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		skybox.bindTexture();
		skybox.draw();
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
	}
	
	/*-----------------------------------------------------------*/

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


// TODO : Proper Texture loading
GLuint loadTexture(const char* file) {
	GLuint texture;
	glGenTextures(1, &texture);
	// glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	int imWidth, imHeight, imNrChannels;
	unsigned char* data = stbi_load(file, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_set_flip_vertically_on_load(false);
	stbi_image_free(data);

	return texture;
}