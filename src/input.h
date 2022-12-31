#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

#include "camera.h"


class InputHandler
{
public:
	enum ScrollType {
		ZOOM,
		SPEED,
		SENSITIVITY,
	};

	ScrollType scroll = ZOOM;
	bool cursorEnabled = false;

	double lastMouseX;
	double lastMouseY;

	Camera* camera;

	// InputHandler(Camera* camera) {
	// 	this->camera = camera;
	// }

	void processInput(GLFWwindow* window, double deltaTime) {
		// Window
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		
		// if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		// 	cursorEnabled = !cursorEnabled;
		// 	// TODO : set with GL
		// }
		
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

		// Scroll and maintain ALT to change speed or CTRL to change sensitivity
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			scroll = SENSITIVITY;
		else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			scroll = SPEED;
		else
			scroll = ZOOM;
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (cursorEnabled || !camera) return;

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
