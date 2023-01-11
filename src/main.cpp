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
void setupLightShader(Shader& multiplelightingShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position, glm::vec3 lightPos, float far_plane);

void renderCube();
bool shadows = true;
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
	GLFWwindow* window = glfwCreateWindow(width, height, "Late night 8 Ball pool", nullptr, nullptr);
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
    // configure depth map FBO
    // -----------------------
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


	// Loading shaders

    Shader shader(PATH_TO_SHADERS "/refraction.vert",
				  PATH_TO_SHADERS "/refraction.frag");
	
	Shader cubeMapShader(PATH_TO_SHADERS "/cubeMap.vert", 
						 PATH_TO_SHADERS "/cubeMap.frag");

	Shader simpleShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/simple.frag");

    Shader lightShader(PATH_TO_SHADERS "/advanced_lighting.vert",
                       PATH_TO_SHADERS "/advanced_lighting.frag");

    Shader lightingShader(PATH_TO_SHADERS "/light_casters.vert",
                          PATH_TO_SHADERS "/light_casters.frag");

    Shader multiplelightingShader(PATH_TO_SHADERS "/multiple_lights.vert",
                                  PATH_TO_SHADERS "/multiple_lights.frag");

	Shader windowShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/window.frag");
						
	Shader mirrorShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/mirror.frag");

	Shader shadowShader(PATH_TO_SHADERS "/point_shadows.vert",
                        PATH_TO_SHADERS "/point_shadows.frag");

    Shader simpleDepthShader(PATH_TO_SHADERS "/point_shadows_depth.vert",
                             PATH_TO_SHADERS "/point_shadows_depth.frag",
                             PATH_TO_SHADERS "/point_shadows_depth.gs");

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

	Mesh wall_mesh = Mesh(PATH_TO_OBJECTS "/plane.obj", true);
	// Entity wall(wall_mesh, Texture(PATH_TO_TEXTURE "/room/woodplanks.jpg"), Texture(PATH_TO_TEXTURE "/room/wall_normalMap.jpg"));
	Entity wall(wall_mesh, Texture(PATH_TO_TEXTURE "/room/woodplanks.jpg"), Texture(PATH_TO_TEXTURE "/room/mud.jpg"));
	wall.transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.1, 0.0f));
	// wall.transform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
	wall.transform = glm::rotate(wall.transform, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
	// wall.transform = glm::rotate(wall.transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// wall.transform = glm::rotate(wall.transform, glm::radians(23.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Scene
	RoomScene room(skybox);

	inputHandler.poolGame = &(room.poolGame);

    Camera camera(glm::vec3(-2.0f, 2.5f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -30.0f, -30.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	inputHandler.camera = &camera;

	// lighting info
    // -------------
	glm::vec3 lightPos(0.0f, 2.0f, 0.0f); 
	glm::vec3 light_pos = glm::vec3(0.5, 1.5, 0.0);  // Unused
	

	//Rendering
	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;
    
	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);

	// shader configuration
    // --------------------

    shadowShader.use();
    // shadowShader.setInteger("depthMap", 1);
    shadowShader.setInteger("depthMap", 2);
	shadowShader.setVector3f("materialColour", materialColour);
    //shadowShader.setInteger("diffuseTexture", 0);
    // shadowShader.setFloat("shininess", 32.0f);

    multiplelightingShader.use();
    multiplelightingShader.setVector3f("materialColour", materialColour);
    multiplelightingShader.setFloat("shininess", 32.0f);

	windowShader.use();
	windowShader.setFloat("refractionIndice", 1.0);

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

	// 0. create depth cubemap transformation matrices
	// -----------------------------------------------
	// float near_plane = 1.0f;
	float near_plane = 0.1f;
	float far_plane = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
	std::vector<glm::mat4> shadowTransforms = createShadowTransforms(shadowProj, lightPos);

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


	// glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	// glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	// glClear(GL_DEPTH_BUFFER_BIT);
	simpleDepthShader.use();
	for (unsigned int i = 0; i < 6; ++i)
		simpleDepthShader.setMatrix4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	simpleDepthShader.setFloat("far_plane", far_plane);
	simpleDepthShader.setVector3f("lightPos", lightPos);

	// glm::mat4 model = glm::mat4(1.0f);
	// model = glm::scale(model, glm::vec3(5.0f));
	// simpleDepthShader.setMatrix4("M", model);
	// glDisable(GL_CULL_FACE);
	// renderCube();
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);


	// room.poolGame.draw(simpleDepthShader);
	// room.drawDepthMap(simpleDepthShader);
	
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);


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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE); 

        // 1. render scene to depth cubemap
        // --------------------------------

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();

		room.drawDepthMap(simpleDepthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// shadowShader.use();
		// // set lighting uniforms
		// shadowShader.setVector3f("lightPos", lightPos);
		// shadowShader.setVector3f("viewPos", camera.Position);
		// shadowShader.setInteger("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
		// shadowShader.setFloat("far_plane", far_plane);
		// shadowShader.setMatrix4("V", view);
		// shadowShader.setMatrix4("P", perspective);

		glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		/*
        glm::mat4 modele = glm::mat4(1.0f);
        modele = glm::scale(modele, glm::vec3(5.0f));
        shadowShader.setMatrix4("M", modele);
        // glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        shadowShader.setInteger("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        shadowShader.setInteger("reverse_normals", 0); // and of course disable it
        // glEnable(GL_CULL_FACE);
		*/
        
		setupLightShader(multiplelightingShader, perspective, view, camera.Position, lightPos, far_plane);

		glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);		
		room.drawRoom(multiplelightingShader, windowShader, perspective, view, camera.Position);
		glDisable(GL_CULL_FACE);

		simpleShader.use();
		simpleShader.setMatrix4("V", view);
		simpleShader.setMatrix4("P", perspective);
		simpleShader.setVector3f("u_view_pos", camera.Position);
		wall.draw(simpleShader);

		// Sky
		glDepthFunc(GL_LEQUAL);
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		skybox.bindTexture();
		skybox.draw();
		glDepthFunc(GL_LESS);
		
		room.drawMirroredRoom(multiplelightingShader, windowShader, mirrorShader, perspective, view, camera.Position);
		
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


void setupLightShader(Shader& multiplelightingShader, glm::mat4 perspective, glm::mat4 view, glm::vec3 position, glm::vec3 lightPos, float far_plane) {
	multiplelightingShader.use();	
	multiplelightingShader.setInteger("depthMap", 2);	
	multiplelightingShader.setFloat("far_plane", far_plane);	
	multiplelightingShader.setMatrix4("V", view);
	multiplelightingShader.setMatrix4("P", perspective);
	multiplelightingShader.setVector3f("u_view_pos", position);
	multiplelightingShader.setVector3f("lightPos", lightPos);
	// multiplelightingShader.setFloat("shininess", 32.0f);

	glm::vec3 testlums1 = glm::vec3(0.5, 1.5, 0.0);
	glm::vec3 testlums2 = glm::vec3(-0.5, 1.5, 0.0);
	glm::vec3 testlums3 = glm::vec3(-1.0, 1.5, 0.0);
	glm::vec3 testlums4 = glm::vec3(1.0, 1.5, 0.0);
	glm::vec3 testdir = glm::vec3(0.0, 0.5, 2.0);

	// directional light
	multiplelightingShader.setVector3f("dirLight.direction", testdir);
	multiplelightingShader.setVector3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	multiplelightingShader.setVector3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	multiplelightingShader.setVector3f("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	multiplelightingShader.setVector3f("pointLights[0].light_pos", testlums1);
	multiplelightingShader.setVector3f("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	multiplelightingShader.setVector3f("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	multiplelightingShader.setVector3f("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setFloat("pointLights[0].constant", 1.0f);
	multiplelightingShader.setFloat("pointLights[0].linear", 0.09f);
	multiplelightingShader.setFloat("pointLights[0].quadratic", 0.032f);
	/*
	// point light 2
	multiplelightingShader.setVector3f("pointLights[1].light_pos", testlums2);
	multiplelightingShader.setVector3f("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	multiplelightingShader.setVector3f("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	multiplelightingShader.setVector3f("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setFloat("pointLights[1].constant", 1.0f);
	multiplelightingShader.setFloat("pointLights[1].linear", 0.09f);
	multiplelightingShader.setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	multiplelightingShader.setVector3f("pointLights[2].light_pos", testlums3);
	multiplelightingShader.setVector3f("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	multiplelightingShader.setVector3f("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	multiplelightingShader.setVector3f("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setFloat("pointLights[2].constant", 1.0f);
	multiplelightingShader.setFloat("pointLights[2].linear", 0.09f);
	multiplelightingShader.setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	multiplelightingShader.setVector3f("pointLights[3].light_pos", testlums4);
	multiplelightingShader.setVector3f("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	multiplelightingShader.setVector3f("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	multiplelightingShader.setVector3f("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setFloat("pointLights[3].constant", 1.0f);
	multiplelightingShader.setFloat("pointLights[3].linear", 0.09f);
	multiplelightingShader.setFloat("pointLights[3].quadratic", 0.032f);
	*/
	// spotLight
	//multiplelightingShader.setVector3f("spotLight.light_pos", camera.Position);
	//multiplelightingShader.setVector3f("spotLight.direction", camera.Front);

	glm::vec3 testspotpos = glm::vec3(0.0, 1.0, -2.0);
	glm::vec3 testspotdir = glm::vec3(0.0, -1.0, 0.0);
	multiplelightingShader.setVector3f("spotLight.light_pos", testspotpos);
	multiplelightingShader.setVector3f("spotLight.direction", testspotdir);

	multiplelightingShader.setVector3f("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	multiplelightingShader.setVector3f("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setVector3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
	multiplelightingShader.setFloat("spotLight.constant", 1.0f);
	multiplelightingShader.setFloat("spotLight.linear", 0.09f);
	multiplelightingShader.setFloat("spotLight.quadratic", 0.032f);
	multiplelightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	multiplelightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}



// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
