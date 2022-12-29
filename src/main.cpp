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
#include "object.h"
#include "input.h"
#include "debug.h"
#include "skybox.h"

const int width = 500;
const int height = 500;


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

	char sourceV[] = PATH_TO_SHADERS "/refraction.vert";
	char sourceF[] = PATH_TO_SHADERS "/refraction.frag";
	Shader shader = Shader(sourceV, sourceF);
	
	char sourceVCubeMap[] = PATH_TO_SHADERS "/cubeMap.vert";
	char sourceFCubeMap[] = PATH_TO_SHADERS "/cubeMap.frag";
	Shader cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap);

	char path[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	Object sphere1(path);
	sphere1.makeObject(shader);

	// --------------- TODO : Proper Texture loading ----------------------
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	int imWidth, imHeight, imNrChannels;
	char file[] = PATH_TO_TEXTURE "/pool_table/colorMap.png";
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


	// -------------------------------------


	char sourceVSimple[] = PATH_TO_SHADERS "/simple.vert";
	char sourceFSimple[] = PATH_TO_SHADERS "/simple.frag";
	Shader simpleShader = Shader(sourceVSimple, sourceFSimple);

	char tablePath[] = PATH_TO_OBJECTS "/pool_table.obj";
	Object pool_table(tablePath);
	pool_table.makeObject(simpleShader);


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

	Skybox skybox(pathToCubeMap, facesToLoad , pathCube, cubeMapShader);

	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};

    Camera camera(glm::vec3(0.0, 0.0, 0.1));

	// glm::vec3 light_pos = glm::vec3(1.0, 2.0, 1.5);
	glm::vec3 light_pos = glm::vec3(0.0, 5.0, -2.0);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, -2.0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

	glm::mat4 inverseModel = glm::transpose(glm::inverse(model));

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;

	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);

	//Rendering

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

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		processInput(window, camera);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		simpleShader.use();

		simpleShader.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		simpleShader.setMatrix4("M", model);
		simpleShader.setMatrix4("itM", inverseModel);
		simpleShader.setMatrix4("V", view);
		simpleShader.setMatrix4("P", perspective);
		simpleShader.setVector3f("u_view_pos", camera.Position);
		pool_table.draw();


		shader.use();

		shader.setMatrix4("M", model);
		shader.setMatrix4("itM", inverseModel);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		
		skybox.bindTexture();
		// cubeMapShader.setInteger("cubemapTexture", 0);

		sphere1.draw();

		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		// cubeMapShader.setInteger("cubemapTexture", 0);
		skybox.draw();
		glDepthFunc(GL_LESS);

		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}