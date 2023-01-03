#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

#include "camera.h"
#include "billiard.h"


class InputHandler
{
public:
	enum ScrollType {
		ZOOM,
		SPEED,
		SENSITIVITY,
	};

	ScrollType scroll = ZOOM;

	bool cursorKeyPressed = false;
	bool cursorEnabled = false;

	bool firstMouse = true;
	double lastMouseX = 0;
	double lastMouseY = 0;

	Camera* camera;
	PoolGame* poolGame;

	void processInput(GLFWwindow* window, double deltaTime) {
		// Window
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		
		// Press Right CTRL to switch between normal cursor and mouse camera aim
		if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS && !cursorKeyPressed) {
			cursorKeyPressed = true;
			firstMouse = true;
			cursorEnabled = !cursorEnabled;
			glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_RELEASE) {
			cursorKeyPressed = false;
		}
		
		processCameraInput(window, deltaTime);
		processPoolInput(window, deltaTime);
	}

	void processCameraInput(GLFWwindow* window, double deltaTime) {
		// Camera Position
		if (!camera) return;

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(RIGHT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(BACKWARD, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera->ProcessKeyboardMovement(DOWN, deltaTime);

		// Camera Rotation (Keyboard)
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			camera->ProcessKeyboardRotation(1.0, 0.0, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			camera->ProcessKeyboardRotation(-1.0, 0.0, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			camera->ProcessKeyboardRotation(0.0, 1.0, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			camera->ProcessKeyboardRotation(0.0, -1.0, deltaTime);

		// Camera Parameters
		// Press R to reset properties (Speed, Mouse sensitivity, Zoom)
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			camera->ResetProperties();

		// Scroll and maintain CTRL to change speed or ALT to change sensitivity
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			scroll = SENSITIVITY;
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			scroll = SPEED;
		else
			scroll = ZOOM;
	}

	void processPoolInput(GLFWwindow* window, double deltaTime) {
		if (!poolGame) return;

		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
			poolGame->resetCueBall();
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (cursorEnabled || !camera) return;
		
		// Avoid using last position the first time
		if (firstMouse) {
			lastMouseX = xpos;
        	lastMouseY = ypos;
        	firstMouse = false;
			return;
		}

		float xoffset = xpos - lastMouseX;
		float yoffset = lastMouseY - ypos; // reversed since y-coordinates go from bottom to top
		lastMouseX = xpos;
		lastMouseY = ypos;
		
		camera->ProcessMouseMovement(xoffset, yoffset);
	}
	
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (!camera) return;

		switch (scroll)
		{
		case ZOOM:
			camera->ProcessZoomScroll(yoffset);
			break;
		case SENSITIVITY:
			camera->ProcessSensitivityScroll(yoffset);
			break;
		case SPEED:
			camera->ProcessSpeedScroll(yoffset);
			break;
		}
	}
};

#endif /* INPUT_H */
