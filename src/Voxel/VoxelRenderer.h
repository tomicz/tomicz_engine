#pragma once

#include "World.h"
#include "../Camera.h"
#include <unordered_map>

// Forward declarations
class Window;

// Voxel renderer class
class VoxelRenderer {
public:
    VoxelRenderer(Window* window);
    ~VoxelRenderer();
    
    // Initialize the renderer
    bool init();
    
    // Render the world
    void render(World* world, const Camera& camera);
    
    // Update chunk meshes
    void updateChunkMeshes(World* world);
    
private:
    // Window reference
    Window* m_window;
    
    // Implementation details are hidden in the .mm file
    struct Impl;
    Impl* m_impl;
    
    // Create chunk mesh
    void createChunkMesh(Chunk* chunk);
    
    // Render chunk
    void renderChunk(Chunk* chunk, const Camera& camera);
    
    // Load texture atlas
    bool loadTextureAtlas();
}; 