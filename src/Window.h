#pragma once

#include <string>
#include <functional>

// Forward declarations
struct GLFWwindow;

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Delete copy constructor and assignment operator
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Initialize the window
    bool init();

    // Check if window should close
    bool shouldClose() const;

    // Update window (poll events)
    void update();

    // Get native window handle
    void* getNativeWindow() const;

    // Get GLFW window handle
    GLFWwindow* getGLFWWindow() const;

    // Get window dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    int m_width;
    int m_height;
    std::string m_title;
    GLFWwindow* m_window;

    // Initialize GLFW (static to ensure it's only done once)
    static bool initGLFW();
}; 