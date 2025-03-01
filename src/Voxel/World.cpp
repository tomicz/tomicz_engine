#include "World.h"
#include <cmath>

// Constructor
World::World() {
}

// Destructor
World::~World() {
    // Delete all chunks
    for (auto& pair : m_chunks) {
        delete pair.second;
    }
    m_chunks.clear();
}

// Get chunk at position
Chunk* World::getChunk(int x, int z) {
    ChunkPosition position = {x, z};
    
    // Check if chunk exists
    auto it = m_chunks.find(position);
    if (it != m_chunks.end()) {
        return it->second;
    }
    
    // Create new chunk
    return createChunk(x, z);
}

// Get block at world position
BlockType World::getBlock(int x, int y, int z) {
    // Convert world position to chunk position
    int localX, localY, localZ;
    worldToLocalPosition(x, y, z, localX, localY, localZ);
    
    // Get chunk
    ChunkPosition chunkPos = worldToChunkPosition(x, z);
    Chunk* chunk = getChunk(chunkPos.x, chunkPos.z);
    
    // Get block
    return chunk->getBlock(localX, localY, localZ);
}

// Set block at world position
void World::setBlock(int x, int y, int z, BlockType type) {
    // Convert world position to chunk position
    int localX, localY, localZ;
    worldToLocalPosition(x, y, z, localX, localY, localZ);
    
    // Get chunk
    ChunkPosition chunkPos = worldToChunkPosition(x, z);
    Chunk* chunk = getChunk(chunkPos.x, chunkPos.z);
    
    // Set block
    chunk->setBlock(localX, localY, localZ, type);
    
    // Mark neighboring chunks as dirty if the block is on the edge
    if (localX == 0) {
        Chunk* neighbor = getChunk(chunkPos.x - 1, chunkPos.z);
        neighbor->setDirty(true);
    } else if (localX == CHUNK_SIZE - 1) {
        Chunk* neighbor = getChunk(chunkPos.x + 1, chunkPos.z);
        neighbor->setDirty(true);
    }
    
    if (localZ == 0) {
        Chunk* neighbor = getChunk(chunkPos.x, chunkPos.z - 1);
        neighbor->setDirty(true);
    } else if (localZ == CHUNK_SIZE - 1) {
        Chunk* neighbor = getChunk(chunkPos.x, chunkPos.z + 1);
        neighbor->setDirty(true);
    }
}

// Update chunks around player
void World::updateChunks(const glm::vec3& playerPosition, int renderDistance) {
    // Convert player position to chunk position
    int playerChunkX = static_cast<int>(std::floor(playerPosition.x / CHUNK_SIZE));
    int playerChunkZ = static_cast<int>(std::floor(playerPosition.z / CHUNK_SIZE));
    
    // Load chunks within render distance
    for (int z = -renderDistance; z <= renderDistance; z++) {
        for (int x = -renderDistance; x <= renderDistance; x++) {
            int chunkX = playerChunkX + x;
            int chunkZ = playerChunkZ + z;
            
            // Get or create chunk
            getChunk(chunkX, chunkZ);
        }
    }
    
    // TODO: Unload chunks outside render distance
}

// Get dirty chunks (need mesh update)
std::vector<Chunk*> World::getDirtyChunks() {
    std::vector<Chunk*> dirtyChunks;
    
    for (auto& pair : m_chunks) {
        if (pair.second->isDirty()) {
            dirtyChunks.push_back(pair.second);
        }
    }
    
    return dirtyChunks;
}

// Convert world position to chunk position
ChunkPosition World::worldToChunkPosition(int x, int z) {
    return {
        static_cast<int>(std::floor(static_cast<float>(x) / CHUNK_SIZE)),
        static_cast<int>(std::floor(static_cast<float>(z) / CHUNK_SIZE))
    };
}

// Convert world position to local chunk position
void World::worldToLocalPosition(int worldX, int worldY, int worldZ, int& localX, int& localY, int& localZ) {
    ChunkPosition chunkPos = worldToChunkPosition(worldX, worldZ);
    
    localX = worldX - chunkPos.x * CHUNK_SIZE;
    localY = worldY;
    localZ = worldZ - chunkPos.z * CHUNK_SIZE;
    
    // Handle negative coordinates
    if (worldX < 0 && localX != 0) localX += CHUNK_SIZE;
    if (worldZ < 0 && localZ != 0) localZ += CHUNK_SIZE;
}

// Create chunk at position
Chunk* World::createChunk(int x, int z) {
    ChunkPosition position = {x, z};
    
    // Create new chunk
    Chunk* chunk = new Chunk(x, z);
    m_chunks[position] = chunk;
    
    // Generate terrain
    chunk->generateTerrain();
    
    return chunk;
} 