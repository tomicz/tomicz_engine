#include "Window.h"
#include "Renderer/MetalRenderer.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Create window
    std::unique_ptr<Window> window(new Window(800, 600, "Tomicz Engine"));
    
    if (!window->init()) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return 1;
    }
    
    // Create renderer
    std::unique_ptr<MetalRenderer> renderer(new MetalRenderer(window.get()));
    
    if (!renderer->init()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return 1;
    }
    
    std::cout << "Tomicz Engine initialized successfully!" << std::endl;
    
    // Main loop
    while (!window->shouldClose()) {
        // Update window (poll events)
        window->update();
        
        // Render frame
        renderer->render();
    }
    
    std::cout << "Shutting down Tomicz Engine..." << std::endl;
    
    return 0;
} 