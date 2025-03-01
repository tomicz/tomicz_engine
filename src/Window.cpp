#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

// Static initialization flag for GLFW
static bool s_GLFWInitialized = false;

bool Window::initGLFW() {
    if (s_GLFWInitialized) {
        return true;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    s_GLFWInitialized = true;
    return true;
}

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title), m_window(nullptr) {
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }

    // Only terminate GLFW if this is the last window
    if (s_GLFWInitialized) {
        glfwTerminate();
        s_GLFWInitialized = false;
    }
}

bool Window::init() {
    if (!initGLFW()) {
        return false;
    }

    // Configure GLFW for Metal
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

    // Create the window
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return false;
    }

    // Get the actual window size (may be different due to high DPI)
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    
    return true;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::update() {
    glfwPollEvents();
}

void* Window::getNativeWindow() const {
#if defined(__APPLE__)
    return (void*)glfwGetCocoaWindow(m_window);
#else
    return nullptr;
#endif
}

GLFWwindow* Window::getGLFWWindow() const {
    return m_window;
} 