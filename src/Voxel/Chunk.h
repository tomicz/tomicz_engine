#pragma once

#include "Block.h"
#include <array>
#include <vector>
#include <glm/glm.hpp>

// Chunk dimensions
constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE;

// Chunk position
struct ChunkPosition {
    int x;
    int z;
    
    bool operator==(const ChunkPosition& other) const {
        return x == other.x && z == other.z;
    }
    
    struct Hash {
        size_t operator()(const ChunkPosition& pos) const {
            return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.z) << 1);
        }
    };
};

// Vertex structure for chunk mesh
struct ChunkVertex {
    float position[3];
    float texCoord[2];
    float normal[3];
    float color[4];
};

// Chunk class
class Chunk {
public:
    Chunk(int x, int z);
    ~Chunk();
    
    // Get block at position
    BlockType getBlock(int x, int y, int z) const;
    
    // Set block at position
    void setBlock(int x, int y, int z, BlockType type);
    
    // Check if position is valid
    bool isValidPosition(int x, int y, int z) const;
    
    // Get chunk position
    const ChunkPosition& getPosition() const { return m_position; }
    
    // Generate mesh
    void generateMesh();
    
    // Get mesh vertices
    const std::vector<ChunkVertex>& getVertices() const { return m_vertices; }
    
    // Get mesh indices
    const std::vector<uint32_t>& getIndices() const { return m_indices; }
    
    // Check if mesh is dirty (needs to be regenerated)
    bool isDirty() const { return m_dirty; }
    
    // Set mesh dirty flag
    void setDirty(bool dirty) { m_dirty = dirty; }
    
    // Generate terrain
    void generateTerrain();
    
private:
    // Chunk position
    ChunkPosition m_position;
    
    // Blocks data
    std::array<BlockType, CHUNK_VOLUME> m_blocks;
    
    // Mesh data
    std::vector<ChunkVertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    // Dirty flag
    bool m_dirty;
    
    // Add face to mesh
    void addFace(BlockType type, BlockFace face, int x, int y, int z);
    
    // Check if face is visible
    bool isFaceVisible(int x, int y, int z, BlockFace face) const;
    
    // Get block at position (including from neighboring chunks)
    BlockType getBlockWorld(int x, int y, int z) const;
}; 