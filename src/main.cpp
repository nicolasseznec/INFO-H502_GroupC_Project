// Some parts of the code were taken from https://learnopengl.com/


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


std::vector<glm::mat4> createShadowTransforms(glm::mat4 shadowProj, glm::vec3 lightPos);
void setupLightShader(Shader& multiplelightingShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position, glm::vec3 lightPos, float far_plane, bool enabledLights);

// const int width = 800;
// const int height = 800;
const int width = 1000;
const int height = 1000;



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
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "8-Ball Game", nullptr, nullptr);
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

	glEnable(GL_MULTISAMPLE); 
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef NDEBUG
    setupDebug();
#endif

	/*-----------------------------------------------------------*/
    // configure depth map FBO for Shadows
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
	// attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float near_plane = 0.1f;
	float far_plane = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);

	// lighting info
    // -------------
	glm::vec3 lightPos(0.0f, 2.0f, 0.0f);
	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);

	// Loading shaders
    // -------------
	Shader cubeMapShader(PATH_TO_SHADERS "/cubeMap.vert", 
						 PATH_TO_SHADERS "/cubeMap.frag");

    Shader multiplelightingShader(PATH_TO_SHADERS "/genericLighting.vert",
                                  PATH_TO_SHADERS "/genericLighting.frag");

	Shader windowShader(PATH_TO_SHADERS "/genericLighting.vert", 
						PATH_TO_SHADERS "/window.frag");
						
	Shader mirrorShader(PATH_TO_SHADERS "/genericLighting.vert", 
						PATH_TO_SHADERS "/mirror.frag");

    Shader simpleDepthShader(PATH_TO_SHADERS "/point_shadows_depth.vert",
                             PATH_TO_SHADERS "/point_shadows_depth.frag",
                             PATH_TO_SHADERS "/point_shadows_depth.gs");
	
	Shader imageShader(PATH_TO_SHADERS "/image.vert",
				  	   PATH_TO_SHADERS "/image.frag");

	Shader lampShader(PATH_TO_SHADERS "/genericLighting.vert",
					   PATH_TO_SHADERS "/lamp.frag");

	// shader configuration
    // --------------------
    multiplelightingShader.use();
    multiplelightingShader.setVector3f("materialColour", materialColour);
    multiplelightingShader.setFloat("shininess", 32.0f);

	// Skybox
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

	// Scene
	RoomScene room(skybox);

    Camera camera(glm::vec3(-2.0f, 2.5f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -30.0f, -30.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	
	inputHandler.poolGame = &(room.poolGame);
	inputHandler.camera = &camera;
	inputHandler.setupControls();

    /*-----------------------------------------------------------*/

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

		bool enabledLights = inputHandler.enabledLights;
		room.enableLights = enabledLights;
		lightPos = enabledLights ? glm::vec3(0.0f, 2.0f, 0.0f) : glm::vec3(15.0f, 10.0f, 0.0f);

        view = camera.GetViewMatrix();
		perspective = camera.GetProjectionMatrix();

		room.update(deltaTime);

		glfwPollEvents();

		// ------------- Render -------------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE); 

        // Render scene to depth cubemap
        // -----------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
		std::vector<glm::mat4> shadowTransforms = createShadowTransforms(shadowProj, lightPos);
        simpleDepthShader.use();
		for (unsigned int i = 0; i < 6; ++i)
			simpleDepthShader.setMatrix4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		simpleDepthShader.setFloat("far_plane", far_plane);
		simpleDepthShader.setVector3f("lightPos", lightPos);

		room.drawDepthMap(simpleDepthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Render scene 
        // ------------
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        
		setupLightShader(multiplelightingShader, perspective, view, camera.Position, lightPos, far_plane, enabledLights);

		glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);		
		room.drawRoom(multiplelightingShader, windowShader, lampShader, perspective, view, camera.Position);
		glDisable(GL_CULL_FACE);

		// Sky
		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		skybox.draw();
		glDepthFunc(GL_LESS);
		

		// Render mirrored scene 
        // ---------------------
		room.drawMirroredRoom(multiplelightingShader, windowShader, lampShader, mirrorShader, perspective, view, camera.Position);
		

		inputHandler.drawControls(imageShader);
		glfwSwapBuffers(window);
	}
	
	/*-----------------------------------------------------------*/

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


std::vector<glm::mat4> createShadowTransforms(glm::mat4 shadowProj, glm::vec3 lightPos) {
	std::vector<glm::mat4> shadowTransforms = {
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
	return shadowTransforms;
}


void setupLightShader(Shader& multiplelightingShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position, glm::vec3 lightPos, float far_plane, bool enableLights) {
	multiplelightingShader.use();
	multiplelightingShader.setBool("useNormalMap", false);
	multiplelightingShader.setInteger("depthMap", 2);
	multiplelightingShader.setFloat("far_plane", far_plane);
	multiplelightingShader.setMatrix4("V", view);
	multiplelightingShader.setMatrix4("P", perspective);
	multiplelightingShader.setVector3f("u_view_pos", position);
	multiplelightingShader.setVector3f("lightPos", lightPos);


    // directional light (outside light)
    glm::vec3 testdir = glm::vec3(-1.0f, -0.5f, 0.0f);
    multiplelightingShader.setBool("dirLights[0].enabled", !enableLights);
    multiplelightingShader.setVector3f("dirLights[0].direction", testdir);
    multiplelightingShader.setVector3f("dirLights[0].ambient", 0.06f, 0.06f, 0.06f);
    multiplelightingShader.setVector3f("dirLights[0].diffuse", 0.4f, 0.4f, 0.4f);
    multiplelightingShader.setVector3f("dirLights[0].specular", 0.5f, 0.5f, 0.5f);


    //Point light
    glm::vec3 testlums1 = glm::vec3(0.0, 2.8, -0.01);
    multiplelightingShader.setBool("pointLights[0].enabled", enableLights);
    multiplelightingShader.setVector3f("pointLights[0].light_pos", testlums1);
	multiplelightingShader.setVector3f("pointLights[0].ambient", 0.24f, 0.24f, 0.24f);
    multiplelightingShader.setVector3f("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    multiplelightingShader.setVector3f("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    multiplelightingShader.setFloat("pointLights[0].constant", 1.0f);
    multiplelightingShader.setFloat("pointLights[0].linear", 0.09f);
    multiplelightingShader.setFloat("pointLights[0].quadratic", 0.032f);
	
	
    //Spotlight 1 (down)
    glm::vec3 testspotpos = glm::vec3(0.0, 2.92, 0.0);
    glm::vec3 testspotdir = glm::vec3(0.0, -1.0, 0.0);
	multiplelightingShader.setBool("spotLights[0].enabled", enableLights);
    multiplelightingShader.setVector3f("spotLights[0].light_pos", testspotpos);
    multiplelightingShader.setVector3f("spotLights[0].direction", testspotdir);
    multiplelightingShader.setVector3f("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
    multiplelightingShader.setVector3f("spotLights[0].diffuse", 0.7f, 0.7f, 0.7f);
    multiplelightingShader.setVector3f("spotLights[0].specular", 0.6f, 0.6f, 0.6f);
    multiplelightingShader.setFloat("spotLights[0].constant", 1.0f);
    multiplelightingShader.setFloat("spotLights[0].linear", 0.09f);
    multiplelightingShader.setFloat("spotLights[0].quadratic", 0.032f);
	multiplelightingShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(17.0f)));
    multiplelightingShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(25.0f)));
}