cmake_minimum_required(VERSION 3.15)
project(tomicz_engine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required frameworks on macOS
find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(METAL_LIBRARY Metal REQUIRED)
find_library(METALKIT_LIBRARY MetalKit REQUIRED)
find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)

# GLFW setup
find_package(glfw3 REQUIRED)

# Set source files
set(SOURCES
    src/main.cpp
    src/Window.cpp
    src/Renderer/MetalRenderer.mm
)

# Set header files
set(HEADERS
    src/Window.h
    src/Renderer/MetalRenderer.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${COCOA_LIBRARY}
    ${METAL_LIBRARY}
    ${METALKIT_LIBRARY}
    ${QUARTZCORE_LIBRARY}
    glfw
)

# Set up Metal shader compilation
set(METAL_SHADER_DIR ${CMAKE_SOURCE_DIR}/src/Shaders)
set(METAL_SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Shaders)

# Create output directory for compiled shaders
file(MAKE_DIRECTORY ${METAL_SHADER_OUTPUT_DIR})

# Add custom command to compile Metal shaders
add_custom_command(
    OUTPUT ${METAL_SHADER_OUTPUT_DIR}/Shaders.metallib
    COMMAND xcrun -sdk macosx metal -c ${METAL_SHADER_DIR}/Shaders.metal -o ${METAL_SHADER_OUTPUT_DIR}/Shaders.air
    COMMAND xcrun -sdk macosx metallib ${METAL_SHADER_OUTPUT_DIR}/Shaders.air -o ${METAL_SHADER_OUTPUT_DIR}/Shaders.metallib
    DEPENDS ${METAL_SHADER_DIR}/Shaders.metal
    COMMENT "Compiling Metal shaders"
)

# Add custom target for shaders
add_custom_target(Shaders DEPENDS ${METAL_SHADER_OUTPUT_DIR}/Shaders.metallib)
add_dependencies(${PROJECT_NAME} Shaders)

# Copy the compiled shader library to the build directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${METAL_SHADER_OUTPUT_DIR}/Shaders.metallib $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying Metal shader library to output directory"
) 