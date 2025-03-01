#pragma once

#include <string>

// Forward declarations
class Window;

// Metal renderer class
class MetalRenderer {
public:
    MetalRenderer(Window* window);
    ~MetalRenderer();

    // Delete copy constructor and assignment operator
    MetalRenderer(const MetalRenderer&) = delete;
    MetalRenderer& operator=(const MetalRenderer&) = delete;

    // Initialize the renderer
    bool init();

    // Render a frame
    void render();

private:
    // Window reference
    Window* m_window;

    // Metal implementation details are hidden in the .mm file
    // to avoid exposing Objective-C++ details in the header
    struct Impl;
    Impl* m_impl;

    // Load and compile Metal shaders
    bool loadShaders();
}; 