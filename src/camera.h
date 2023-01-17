#ifndef CAMERA_H
#define CAMERA_H

// Some parts of the code were taken from https://learnopengl.com/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.0f;

const float M_SENSITIVITY = 0.08f;
const float KB_SENSITIVITY = 50.0f;
const float ZOOM = 80.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;

    // camera options
    float MovementSpeed;
    float KeyboardSensitivity;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = YAW, 
        float pitch = PITCH
        ) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(M_SENSITIVITY), KeyboardSensitivity(KB_SENSITIVITY), Zoom(ZOOM)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(M_SENSITIVITY), KeyboardSensitivity(KB_SENSITIVITY), Zoom(ZOOM)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    glm::mat4 GetProjectionMatrix(float ratio=1.0, float near=0.01, float far=50.0)
    {
        return glm::perspective(glm::radians(Zoom), ratio, near, far);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboardMovement(Camera_Movement direction, float deltaTime)
    {
        float velocity = this->MovementSpeed * deltaTime;

        if (direction == FORWARD)
            this->Position += this->Front * velocity;
        if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
        
        if (direction == UP)
            this->Position += this->Up * velocity;
        if (direction == DOWN)
            this->Position -= this->Up * velocity;
            
    }

    void ProcessKeyboardRotation(float YawRot, float PitchRot, float deltaTime, bool constrainPitch = true)
    {
        float velocity = this->KeyboardSensitivity * deltaTime;
        YawRot *= velocity;
        PitchRot *= velocity;

        updateRotation(YawRot, PitchRot, constrainPitch);
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
		float YawRot = xoffset * MouseSensitivity;
		float PitchRot = yoffset * MouseSensitivity;

        updateRotation(YawRot, PitchRot, constrainPitch);;
    }

    void ProcessZoomScroll(float yoffset)
    {
        float factor = yoffset > 0 ? 0.75f : 1.5f;
        Zoom *= factor; 
        if (Zoom < 10.0f)
            Zoom = 10.0f;
        if (Zoom > ZOOM)
            Zoom = ZOOM;
    }

    void ProcessSpeedScroll(float yoffset) {
        float factor = yoffset < 0 ? 0.75f : 1.5f;
        MovementSpeed *= factor;
        if (MovementSpeed < SPEED * 0.1f)
            MovementSpeed = SPEED * 0.1f;
        if (MovementSpeed > SPEED * 10.0f)
            MovementSpeed = SPEED * 10.0f;
    }

    void ProcessSensitivityScroll(float yoffset) {
        float factor = yoffset < 0 ? 0.75f : 1.5f;
        MouseSensitivity *= factor;
        if (MouseSensitivity < M_SENSITIVITY * 0.1f)
            MouseSensitivity = M_SENSITIVITY * 0.1f;
        if (MouseSensitivity > M_SENSITIVITY * 10.0f)
            MouseSensitivity = M_SENSITIVITY * 10.0f;
    }

    void ResetProperties() {
        MovementSpeed = SPEED;
        Zoom = ZOOM;
        MouseSensitivity = M_SENSITIVITY;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }

    void updateRotation(float yawRot, float pitchRot, bool constrainPitch = true) {
        this->Yaw += yawRot;
        this->Pitch += pitchRot;

        if (constrainPitch) this->constrainPitch();
        updateCameraVectors();   
    }

    void constrainPitch() {
        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (this->Pitch > 89.0f)
            this->Pitch = 89.0f;
        if (this->Pitch < -89.0f)
            this->Pitch = -89.0f;      
    }
};

#endif