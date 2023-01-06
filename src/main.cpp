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



unsigned int loadTexture(const char *path);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef NDEBUG
    setupDebug();
#endif

	/*-----------------------------------------------------------*/




    Shader shadowShader(PATH_TO_SHADERS "/3.2.2.point_shadows.vert",
                        PATH_TO_SHADERS "/3.2.2.point_shadows.frag");


    Shader simpleDepthShader(PATH_TO_SHADERS "/3.2.2.point_shadows_depth.vert",
                             PATH_TO_SHADERS "/3.2.2.point_shadows_depth.frag",
                             PATH_TO_SHADERS "/3.2.2.point_shadows_depth.gs");



    unsigned int woodTexture = loadTexture("C:/Users/Mohamed/Desktop/LearnOpenGL/resources/textures/wood.png");


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



    // shader configuration
    // --------------------
    shadowShader.use();
    //shadowShader.setInteger("diffuseTexture", 0);
    shadowShader.setInteger("depthMap", 1);

    // lighting info
    // -------------

    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);



    Shader shader(PATH_TO_SHADERS "/refraction.vert",
				  PATH_TO_SHADERS "/refraction.frag");
	
	Shader cubeMapShader(PATH_TO_SHADERS "/cubeMap.vert", 
						 PATH_TO_SHADERS "/cubeMap.frag");

	Shader simpleShader(PATH_TO_SHADERS "/simple.vert", 
						PATH_TO_SHADERS "/simple.frag");

    Shader lightShader(PATH_TO_SHADERS "/1.advanced_lighting.vert",
                       PATH_TO_SHADERS "/1.advanced_lighting.frag");

    Shader lightingShader(PATH_TO_SHADERS "/5.4.light_casters.vert",
                          PATH_TO_SHADERS "/5.4.light_casters.frag");

    Shader multiplelightingShader(PATH_TO_SHADERS "/6.multiple_lights.vert",
                                  PATH_TO_SHADERS "/6.multiple_lights.frag");

	PoolGame poolGame(
		PATH_TO_OBJECTS "/pool_table.obj",
		PATH_TO_TEXTURE "/pool_table/colorMap.png",
		PATH_TO_OBJECTS "/pool_ball.obj",
		PATH_TO_TEXTURE "/pool_balls/"
	);
	inputHandler.poolGame = &poolGame;

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


    Camera camera(glm::vec3(0.0, 0.0, 0.1));
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();
	inputHandler.camera = &camera;

	// glm::vec3 light_pos = glm::vec3(1.0, 2.0, 1.5);
	glm::vec3 light_pos = glm::vec3(0.0, 5.0, -2.0);








    // model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	/*
	pool_table.transform = glm::translate(pool_table.transform, glm::vec3(0.0, -1.0, -2.0));
	ball.transform = glm::translate(ball.transform, glm::vec3(0.0, 0.0, -2.0));
	ball2.transform = glm::translate(ball2.transform, glm::vec3(-0.5, 0.0, -2.25));
	ball2.transform = glm::rotate(ball2.transform, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	*/

	//Rendering
    /*
	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;
     */

	glm::vec3 materialColour = glm::vec3(0.5f, 0.6, 0.8);



    /*
	simpleShader.use();
	simpleShader.setFloat("shininess", 32.0f);
	simpleShader.setVector3f("materialColour", materialColour);
	simpleShader.setFloat("light.ambient_strength", ambient);
	simpleShader.setFloat("light.diffuse_strength", diffuse);
	simpleShader.setFloat("light.specular_strength", specular);
	simpleShader.setFloat("light.constant", 1.0);
	simpleShader.setFloat("light.linear", 0.14);
	simpleShader.setFloat("light.quadratic", 0.07);
     */




    lightShader.use();
    lightShader.setVector3f("materialColour", materialColour);


    lightingShader.use();
    lightingShader.setVector3f("materialColour", materialColour);
    lightingShader.setFloat("shininess", 32.0f);



    multiplelightingShader.use();
    multiplelightingShader.setVector3f("materialColour", materialColour);
    multiplelightingShader.setFloat("shininess", 32.0f);



    lightShader.setFloat("shininess", 32.0f);
    lightShader.setVector3f("materialColour", materialColour);
    lightShader.setFloat("light.ambient_strength", 0.1);
    lightShader.setFloat("light.diffuse_strength", 0.5);
    lightShader.setFloat("light.specular_strength", 0.8);
    lightShader.setFloat("light.constant", 1.0);
    lightShader.setFloat("light.linear", 0.14);
    lightShader.setFloat("light.quadratic", 0.07);






	shader.use();
	shader.setFloat("refractionIndice", 1.52);



    shadowShader.use();
    shadowShader.setVector3f("materialColour", materialColour);
    //shadowShader.setFloat("shininess", 32.0f);



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
		glfwPollEvents();


		// ------------- Render -------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMatrix4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", far_plane);
        simpleDepthShader.setVector3f("lightPos", lightPos);







        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));
        simpleDepthShader.setMatrix4("M", model);
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        //simpleDepthShader.setInteger("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        //simpleDepthShader.setInteger("reverse_normals", 0); // and of course disable it
        glEnable(GL_CULL_FACE);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        simpleDepthShader.setMatrix4("M", model);
        //renderCube();
        poolGame.draw(simpleDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);








        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadowShader.use();
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        //shadowShader.setMatrix4("projection", projection);
        //shadowShader.setMatrix4("view", view);
        // set lighting uniforms
        shadowShader.setVector3f("lightPos", lightPos);
        shadowShader.setVector3f("viewPos", camera.Position);
        shadowShader.setInteger("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        shadowShader.setFloat("far_plane", far_plane);
        shadowShader.use();
        shadowShader.setMatrix4("V", view);
        shadowShader.setMatrix4("P", perspective);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        glm::mat4 modele = glm::mat4(1.0f);
        modele = glm::scale(modele, glm::vec3(5.0f));
        shadowShader.setMatrix4("M", modele);
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        shadowShader.setInteger("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        shadowShader.setInteger("reverse_normals", 0); // and of course disable it
        glEnable(GL_CULL_FACE);
        // cubes
        modele = glm::mat4(1.0f);
        modele = glm::translate(modele, glm::vec3(4.0f, -3.5f, 0.0));
        modele = glm::scale(modele, glm::vec3(0.5f));
        shadowShader.setMatrix4("M", modele);
        //renderCube();
        poolGame.update(deltaTime);
        poolGame.draw(shadowShader);





        //Initial
        /*
        simpleShader.use();
        simpleShader.setMatrix4("V", view);
        simpleShader.setMatrix4("P", perspective);
        simpleShader.setVector3f("u_view_pos", camera.Position);
        poolGame.update(deltaTime);
        poolGame.draw(simpleShader);
        */


        //Blinn-Phong
        /*
        lightShader.use();
        lightShader.setMatrix4("V", view);
        lightShader.setMatrix4("P", perspective);
        lightShader.setVector3f("u_view_pos", camera.Position);
        lightShader.setVector3f("light.light_pos", light_pos);
        poolGame.update(deltaTime);
        poolGame.draw(lightShader);
        */


        //Single spotlight
        /*
        lightingShader.use();
        lightingShader.setMatrix4("V", view);
        lightingShader.setMatrix4("P", perspective);
        lightingShader.setVector3f("u_view_pos", camera.Position);
        lightingShader.setFloat("shininess", 32.0f);

        glm::vec3 testlums = glm::vec3(-0.5, 2.0, -2.0);
        glm::vec3 testdir = glm::vec3(0.0, -1.0, 0.0);

        lightingShader.setVector3f("light.light_pos", testlums);
        lightingShader.setVector3f("light.direction", testdir);

        //lightingShader.setVector3f("light.light_pos", camera.Position);
        //lightingShader.setVector3f("light.direction", camera.Front);

        lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
        lightingShader.setVector3f("light.ambient", 0.1f, 0.1f, 0.1f);
        // we configure the diffuse intensity slightly higher; the right lighting conditions differ with each lighting method and environment.
        // each environment and lighting type requires some tweaking to get the best out of your environment.
        lightingShader.setVector3f("light.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVector3f("light.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("light.constant", 1.0f);
        lightingShader.setFloat("light.linear", 0.09f);
        lightingShader.setFloat("light.quadratic", 0.032f);

        poolGame.update(deltaTime);
        poolGame.draw(lightingShader);
        */



        /*
        //Multiple sources of light (spotlight, light points and directional light)
        // be sure to activate shader when setting uniforms/drawing objects
        multiplelightingShader.use();
        multiplelightingShader.setMatrix4("V", view);
        multiplelightingShader.setMatrix4("P", perspective);
        multiplelightingShader.setVector3f("u_view_pos", camera.Position);
        multiplelightingShader.setFloat("shininess", 32.0f);

        glm::vec3 testlums1 = glm::vec3(-1.0, 0.0, -2.0);
        glm::vec3 testlums2 = glm::vec3(-0.5, 0.0, -2.0);
        glm::vec3 testlums3 = glm::vec3(0.5, 0.0, -2.0);
        glm::vec3 testlums4 = glm::vec3(1.0, 0.0, -2.0);
        glm::vec3 testdir = glm::vec3(0.0, -1.0, 0.0);

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

        poolGame.update(deltaTime);
        poolGame.draw(multiplelightingShader);
         */

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




// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
