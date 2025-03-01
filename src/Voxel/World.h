#pragma once

#include "Chunk.h"
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

// World class
class World {
public:
    World();
    ~World();
    
    // Get chunk at position
    Chunk* getChunk(int x, int z);
    
    // Get block at world position
    BlockType getBlock(int x, int y, int z);
    
    // Set block at world position
    void setBlock(int x, int y, int z, BlockType type);
    
    // Update chunks around player
    void updateChunks(const glm::vec3& playerPosition, int renderDistance);
    
    // Get all chunks
    const std::unordered_map<ChunkPosition, Chunk*, ChunkPosition::Hash>& getChunks() const { return m_chunks; }
    
    // Get dirty chunks (need mesh update)
    std::vector<Chunk*> getDirtyChunks();
    
    // Convert world position to chunk position
    static ChunkPosition worldToChunkPosition(int x, int z);
    
    // Convert world position to local chunk position
    static void worldToLocalPosition(int worldX, int worldY, int worldZ, int& localX, int& localY, int& localZ);
    
private:
    // Chunks map
    std::unordered_map<ChunkPosition, Chunk*, ChunkPosition::Hash> m_chunks;
    
    // Create chunk at position
    Chunk* createChunk(int x, int z);
}; 