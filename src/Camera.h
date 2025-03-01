#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declarations
struct GLFWwindow;

// Camera class
class Camera {
public:
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);
    ~Camera();
    
    // Update camera
    void update(GLFWwindow* window, float deltaTime);
    
    // Get view matrix
    glm::mat4 getViewMatrix() const;
    
    // Get projection matrix
    glm::mat4 getProjectionMatrix() const;
    
    // Get camera position
    const glm::vec3& getPosition() const { return m_position; }
    
    // Get camera front vector
    const glm::vec3& getFront() const { return m_front; }
    
    // Get camera up vector
    const glm::vec3& getUp() const { return m_up; }
    
    // Get camera right vector
    const glm::vec3& getRight() const { return m_right; }
    
    // Set camera position
    void setPosition(const glm::vec3& position) { m_position = position; }
    
    // Set camera rotation
    void setRotation(float yaw, float pitch);
    
    // Get camera yaw
    float getYaw() const { return m_yaw; }
    
    // Get camera pitch
    float getPitch() const { return m_pitch; }
    
    // Get camera field of view
    float getFov() const { return m_fov; }
    
    // Set camera field of view
    void setFov(float fov) { m_fov = fov; updateProjectionMatrix(); }
    
    // Set aspect ratio
    void setAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; updateProjectionMatrix(); }
    
private:
    // Camera position
    glm::vec3 m_position;
    
    // Camera orientation
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // Euler angles
    float m_yaw;
    float m_pitch;
    
    // Camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_fov;
    
    // Projection matrix parameters
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
    
    // Matrices
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    
    // Mouse position
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;
    
    // Update camera vectors
    void updateCameraVectors();
    
    // Update projection matrix
    void updateProjectionMatrix();
}; 