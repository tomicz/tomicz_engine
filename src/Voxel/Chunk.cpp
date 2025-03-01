#include "Chunk.h"
#include <algorithm>
#include <cmath>

// Noise library for terrain generation
#include "FastNoise.h"

// Face normals
const float FACE_NORMALS[6][3] = {
    { 0.0f,  0.0f,  1.0f}, // Front
    { 0.0f,  0.0f, -1.0f}, // Back
    {-1.0f,  0.0f,  0.0f}, // Left
    { 1.0f,  0.0f,  0.0f}, // Right
    { 0.0f,  1.0f,  0.0f}, // Top
    { 0.0f, -1.0f,  0.0f}  // Bottom
};

// Face vertices (positions relative to block origin)
const float FACE_VERTICES[6][4][3] = {
    // Front face (z+)
    {
        {0.0f, 0.0f, 1.0f}, // Bottom-left
        {1.0f, 0.0f, 1.0f}, // Bottom-right
        {1.0f, 1.0f, 1.0f}, // Top-right
        {0.0f, 1.0f, 1.0f}  // Top-left
    },
    // Back face (z-)
    {
        {1.0f, 0.0f, 0.0f}, // Bottom-left
        {0.0f, 0.0f, 0.0f}, // Bottom-right
        {0.0f, 1.0f, 0.0f}, // Top-right
        {1.0f, 1.0f, 0.0f}  // Top-left
    },
    // Left face (x-)
    {
        {0.0f, 0.0f, 0.0f}, // Bottom-left
        {0.0f, 0.0f, 1.0f}, // Bottom-right
        {0.0f, 1.0f, 1.0f}, // Top-right
        {0.0f, 1.0f, 0.0f}  // Top-left
    },
    // Right face (x+)
    {
        {1.0f, 0.0f, 1.0f}, // Bottom-left
        {1.0f, 0.0f, 0.0f}, // Bottom-right
        {1.0f, 1.0f, 0.0f}, // Top-right
        {1.0f, 1.0f, 1.0f}  // Top-left
    },
    // Top face (y+)
    {
        {0.0f, 1.0f, 1.0f}, // Bottom-left
        {1.0f, 1.0f, 1.0f}, // Bottom-right
        {1.0f, 1.0f, 0.0f}, // Top-right
        {0.0f, 1.0f, 0.0f}  // Top-left
    },
    // Bottom face (y-)
    {
        {0.0f, 0.0f, 0.0f}, // Bottom-left
        {1.0f, 0.0f, 0.0f}, // Bottom-right
        {1.0f, 0.0f, 1.0f}, // Top-right
        {0.0f, 0.0f, 1.0f}  // Top-left
    }
};

// Face texture coordinates
const float FACE_TEX_COORDS[4][2] = {
    {0.0f, 1.0f}, // Bottom-left
    {1.0f, 1.0f}, // Bottom-right
    {1.0f, 0.0f}, // Top-right
    {0.0f, 0.0f}  // Top-left
};

// Constructor
Chunk::Chunk(int x, int z)
    : m_position({x, z}), m_dirty(true) {
    // Initialize blocks to air
    std::fill(m_blocks.begin(), m_blocks.end(), BlockType::Air);
}

// Destructor
Chunk::~Chunk() {
}

// Get block at position
BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!isValidPosition(x, y, z)) {
        return BlockType::Air;
    }
    
    int index = y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    return m_blocks[index];
}

// Set block at position
void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (!isValidPosition(x, y, z)) {
        return;
    }
    
    int index = y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    m_blocks[index] = type;
    m_dirty = true;
}

// Check if position is valid
bool Chunk::isValidPosition(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE;
}

// Generate mesh
void Chunk::generateMesh() {
    // Clear previous mesh
    m_vertices.clear();
    m_indices.clear();
    
    // Iterate through all blocks
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                BlockType type = getBlock(x, y, z);
                
                // Skip air blocks
                if (type == BlockType::Air) {
                    continue;
                }
                
                // Check each face
                for (int face = 0; face < static_cast<int>(BlockFace::Count); face++) {
                    if (isFaceVisible(x, y, z, static_cast<BlockFace>(face))) {
                        addFace(type, static_cast<BlockFace>(face), x, y, z);
                    }
                }
            }
        }
    }
    
    m_dirty = false;
}

// Add face to mesh
void Chunk::addFace(BlockType type, BlockFace face, int x, int y, int z) {
    // Get texture coordinates
    float u, v;
    Block::getTextureCoords(type, face, u, v);
    
    // Get face normal
    const float* normal = FACE_NORMALS[static_cast<int>(face)];
    
    // Get face vertices
    const float (*vertices)[3] = FACE_VERTICES[static_cast<int>(face)];
    
    // Add vertices
    uint32_t indexOffset = static_cast<uint32_t>(m_vertices.size());
    
    for (int i = 0; i < 4; i++) {
        ChunkVertex vertex;
        
        // Position
        vertex.position[0] = vertices[i][0] + static_cast<float>(x);
        vertex.position[1] = vertices[i][1] + static_cast<float>(y);
        vertex.position[2] = vertices[i][2] + static_cast<float>(z);
        
        // Texture coordinates
        vertex.texCoord[0] = FACE_TEX_COORDS[i][0] * 0.25f + u;
        vertex.texCoord[1] = FACE_TEX_COORDS[i][1] * 0.25f + v;
        
        // Normal
        vertex.normal[0] = normal[0];
        vertex.normal[1] = normal[1];
        vertex.normal[2] = normal[2];
        
        // Color (white)
        vertex.color[0] = 1.0f;
        vertex.color[1] = 1.0f;
        vertex.color[2] = 1.0f;
        vertex.color[3] = 1.0f;
        
        m_vertices.push_back(vertex);
    }
    
    // Add indices (two triangles per face)
    m_indices.push_back(indexOffset);
    m_indices.push_back(indexOffset + 1);
    m_indices.push_back(indexOffset + 2);
    
    m_indices.push_back(indexOffset);
    m_indices.push_back(indexOffset + 2);
    m_indices.push_back(indexOffset + 3);
}

// Check if face is visible
bool Chunk::isFaceVisible(int x, int y, int z, BlockFace face) const {
    // Get adjacent block position
    int nx = x, ny = y, nz = z;
    
    switch (face) {
        case BlockFace::Front:  nz++; break;
        case BlockFace::Back:   nz--; break;
        case BlockFace::Left:   nx--; break;
        case BlockFace::Right:  nx++; break;
        case BlockFace::Top:    ny++; break;
        case BlockFace::Bottom: ny--; break;
        default: break;
    }
    
    // Get adjacent block
    BlockType adjacentBlock = getBlockWorld(nx, ny, nz);
    
    // Face is visible if adjacent block is air or transparent
    return adjacentBlock == BlockType::Air || Block::isTransparent(adjacentBlock);
}

// Get block at position (including from neighboring chunks)
BlockType Chunk::getBlockWorld(int x, int y, int z) const {
    // Check if y is out of bounds
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return BlockType::Air;
    }
    
    // Check if block is in this chunk
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
        return getBlock(x, y, z);
    }
    
    // Block is in a neighboring chunk
    // In a real implementation, we would get the block from the neighboring chunk
    // For now, just return air
    return BlockType::Air;
}

// Generate terrain
void Chunk::generateTerrain() {
    // Create noise generator
    FastNoise noise;
    noise.SetNoiseType(FastNoise::SimplexFractal);
    noise.SetSeed(12345);
    noise.SetFrequency(0.01f);
    noise.SetFractalOctaves(4);
    
    // Generate terrain
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            // Calculate world coordinates
            float worldX = static_cast<float>(x + m_position.x * CHUNK_SIZE);
            float worldZ = static_cast<float>(z + m_position.z * CHUNK_SIZE);
            
            // Generate height using noise
            float heightValue = noise.GetNoise(worldX, worldZ);
            int height = static_cast<int>((heightValue + 1.0f) * 32.0f + 64.0f);
            height = std::min(height, CHUNK_HEIGHT - 1);
            
            // Fill blocks
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                BlockType type;
                
                if (y > height) {
                    // Air above ground
                    type = BlockType::Air;
                } else if (y == height) {
                    // Grass on top
                    type = BlockType::Grass;
                } else if (y > height - 4) {
                    // Dirt below grass
                    type = BlockType::Dirt;
                } else {
                    // Stone below dirt
                    type = BlockType::Stone;
                }
                
                setBlock(x, y, z, type);
            }
        }
    }
    
    m_dirty = true;
} 