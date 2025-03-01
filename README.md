# Tomicz Engine

A modern C++ game engine using Metal for rendering on macOS.

## Requirements

- macOS with Metal support
- CMake 3.15 or higher
- GLFW 3.x

## Dependencies

Before building, make sure you have GLFW installed. You can install it using Homebrew:

```bash
brew install glfw
```

## Building

1. Create a build directory:

```bash
mkdir build
cd build
```

2. Configure with CMake:

```bash
cmake ..
```

3. Build the project:

```bash
make
```

## Running

After building, you can run the engine:

```bash
./tomicz_engine
```

You should see a window with a colored square rendered using Metal.

## Project Structure

- `src/` - Source code
  - `main.cpp` - Entry point
  - `Window.h/cpp` - Window management using GLFW
  - `Renderer/` - Rendering code
    - `MetalRenderer.h/mm` - Metal renderer implementation
  - `Shaders/` - Metal shader files
    - `Shaders.metal` - Vertex and fragment shaders

## Features

- Window creation with GLFW
- Metal rendering
- Basic shader support
- Simple square rendering

## Next Steps

- Add input handling
- Implement a scene graph
- Add support for textures
- Create a component-based entity system
- Add physics integration
- Support for multiple rendering backends (OpenGL, Vulkan)
