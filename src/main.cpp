#include "Window.h"
#include "Camera.h"
#include "Voxel/World.h"
#include "Voxel/VoxelRenderer.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <chrono>

int main(int argc, char* argv[]) {
    // Create window
    std::unique_ptr<Window> window(new Window(800, 600, "Tomicz Engine - Voxel Game"));
    
    if (!window->init()) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return 1;
    }
    
    // Create camera
    std::unique_ptr<Camera> camera(new Camera(70.0f, 800.0f / 600.0f, 0.1f, 1000.0f));
    camera->setPosition(glm::vec3(0.0f, 70.0f, 0.0f));
    
    // Create world
    std::unique_ptr<World> world(new World());
    
    // Create voxel renderer
    std::unique_ptr<VoxelRenderer> voxelRenderer(new VoxelRenderer(window.get()));
    
    if (!voxelRenderer->init()) {
        std::cerr << "Failed to initialize voxel renderer!" << std::endl;
        return 1;
    }
    
    std::cout << "Tomicz Engine initialized successfully!" << std::endl;
    
    // Initialize chunks around player
    world->updateChunks(camera->getPosition(), 3);
    
    // Update chunk meshes
    voxelRenderer->updateChunkMeshes(world.get());
    
    // Set up timing
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    // Capture mouse
    GLFWwindow* glfwWindow = window->getGLFWWindow();
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Main loop
    while (!window->shouldClose()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update camera
        camera->update(window->getGLFWWindow(), deltaTime);
        
        // Update chunks around player
        world->updateChunks(camera->getPosition(), 3);
        
        // Update chunk meshes
        voxelRenderer->updateChunkMeshes(world.get());
        
        // Render world
        voxelRenderer->render(world.get(), *camera);
        
        // Update window (poll events)
        window->update();
        
        // Handle escape key to exit
        if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
        }
    }
    
    std::cout << "Shutting down Tomicz Engine..." << std::endl;
    
    return 0;
} 