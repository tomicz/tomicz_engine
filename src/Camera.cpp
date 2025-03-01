#include "Camera.h"
#include <GLFW/glfw3.h>
#include <cmath>

// Constructor
Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : m_position(0.0f, 0.0f, 0.0f),
      m_front(0.0f, 0.0f, -1.0f),
      m_up(0.0f, 1.0f, 0.0f),
      m_right(1.0f, 0.0f, 0.0f),
      m_worldUp(0.0f, 1.0f, 0.0f),
      m_yaw(-90.0f),
      m_pitch(0.0f),
      m_movementSpeed(5.0f),
      m_mouseSensitivity(0.1f),
      m_fov(fov),
      m_aspectRatio(aspectRatio),
      m_nearPlane(nearPlane),
      m_farPlane(farPlane),
      m_lastMouseX(0.0),
      m_lastMouseY(0.0),
      m_firstMouse(true) {
    
    // Initialize view and projection matrices
    updateCameraVectors();
    updateProjectionMatrix();
}

// Destructor
Camera::~Camera() {
}

// Update camera
void Camera::update(GLFWwindow* window, float deltaTime) {
    // Process keyboard input
    float velocity = m_movementSpeed * deltaTime;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_position += m_front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_position -= m_front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_position -= m_right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_position += m_right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        m_position += m_worldUp * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        m_position -= m_worldUp * velocity;
    }
    
    // Process mouse input
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (m_firstMouse) {
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        m_firstMouse = false;
    }
    
    double xOffset = mouseX - m_lastMouseX;
    double yOffset = m_lastMouseY - mouseY; // Reversed since y-coordinates range from bottom to top
    
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;
    
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;
    
    m_yaw += static_cast<float>(xOffset);
    m_pitch += static_cast<float>(yOffset);
    
    // Constrain pitch
    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }
    if (m_pitch < -89.0f) {
        m_pitch = -89.0f;
    }
    
    // Update camera vectors
    updateCameraVectors();
}

// Get view matrix
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

// Get projection matrix
glm::mat4 Camera::getProjectionMatrix() const {
    return m_projectionMatrix;
}

// Set camera rotation
void Camera::setRotation(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = pitch;
    
    // Constrain pitch
    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }
    if (m_pitch < -89.0f) {
        m_pitch = -89.0f;
    }
    
    // Update camera vectors
    updateCameraVectors();
}

// Update camera vectors
void Camera::updateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    // Recalculate the right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
    
    // Update view matrix
    m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
}

// Update projection matrix
void Camera::updateProjectionMatrix() {
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
} 