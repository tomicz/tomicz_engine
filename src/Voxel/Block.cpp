#include "Block.h"

// Initialize static members
std::unordered_map<BlockType, Block::BlockProperties> Block::s_blockProperties;
bool Block::s_initialized = false;

// Initialize block properties
void Block::initBlockProperties() {
    if (s_initialized) return;
    
    // Air
    s_blockProperties[BlockType::Air] = {
        true,   // transparent
        false,  // solid
        false,  // liquid
        "Air",  // name
        {
            {0.0f, 0.0f}, // Front
            {0.0f, 0.0f}, // Back
            {0.0f, 0.0f}, // Left
            {0.0f, 0.0f}, // Right
            {0.0f, 0.0f}, // Top
            {0.0f, 0.0f}  // Bottom
        }
    };
    
    // Grass
    s_blockProperties[BlockType::Grass] = {
        false,  // transparent
        true,   // solid
        false,  // liquid
        "Grass", // name
        {
            {0.0f, 0.0f}, // Front - Side texture
            {0.0f, 0.0f}, // Back - Side texture
            {0.0f, 0.0f}, // Left - Side texture
            {0.0f, 0.0f}, // Right - Side texture
            {0.0f, 0.25f}, // Top - Grass texture
            {0.25f, 0.0f}  // Bottom - Dirt texture
        }
    };
    
    // Dirt
    s_blockProperties[BlockType::Dirt] = {
        false,  // transparent
        true,   // solid
        false,  // liquid
        "Dirt", // name
        {
            {0.25f, 0.0f}, // Front
            {0.25f, 0.0f}, // Back
            {0.25f, 0.0f}, // Left
            {0.25f, 0.0f}, // Right
            {0.25f, 0.0f}, // Top
            {0.25f, 0.0f}  // Bottom
        }
    };
    
    // Stone
    s_blockProperties[BlockType::Stone] = {
        false,  // transparent
        true,   // solid
        false,  // liquid
        "Stone", // name
        {
            {0.5f, 0.0f}, // Front
            {0.5f, 0.0f}, // Back
            {0.5f, 0.0f}, // Left
            {0.5f, 0.0f}, // Right
            {0.5f, 0.0f}, // Top
            {0.5f, 0.0f}  // Bottom
        }
    };
    
    // Sand
    s_blockProperties[BlockType::Sand] = {
        false,  // transparent
        true,   // solid
        false,  // liquid
        "Sand", // name
        {
            {0.75f, 0.0f}, // Front
            {0.75f, 0.0f}, // Back
            {0.75f, 0.0f}, // Left
            {0.75f, 0.0f}, // Right
            {0.75f, 0.0f}, // Top
            {0.75f, 0.0f}  // Bottom
        }
    };
    
    // Water
    s_blockProperties[BlockType::Water] = {
        true,   // transparent
        false,  // solid
        true,   // liquid
        "Water", // name
        {
            {0.0f, 0.25f}, // Front
            {0.0f, 0.25f}, // Back
            {0.0f, 0.25f}, // Left
            {0.0f, 0.25f}, // Right
            {0.0f, 0.25f}, // Top
            {0.0f, 0.25f}  // Bottom
        }
    };
    
    // Wood
    s_blockProperties[BlockType::Wood] = {
        false,  // transparent
        true,   // solid
        false,  // liquid
        "Wood", // name
        {
            {0.25f, 0.25f}, // Front
            {0.25f, 0.25f}, // Back
            {0.25f, 0.25f}, // Left
            {0.25f, 0.25f}, // Right
            {0.5f, 0.25f},  // Top - Top texture
            {0.5f, 0.25f}   // Bottom - Bottom texture
        }
    };
    
    // Leaves
    s_blockProperties[BlockType::Leaves] = {
        true,   // transparent (semi-transparent)
        true,   // solid
        false,  // liquid
        "Leaves", // name
        {
            {0.75f, 0.25f}, // Front
            {0.75f, 0.25f}, // Back
            {0.75f, 0.25f}, // Left
            {0.75f, 0.25f}, // Right
            {0.75f, 0.25f}, // Top
            {0.75f, 0.25f}  // Bottom
        }
    };
    
    s_initialized = true;
}

// Get block properties
bool Block::isTransparent(BlockType type) {
    if (!s_initialized) initBlockProperties();
    return s_blockProperties[type].transparent;
}

bool Block::isSolid(BlockType type) {
    if (!s_initialized) initBlockProperties();
    return s_blockProperties[type].solid;
}

bool Block::isLiquid(BlockType type) {
    if (!s_initialized) initBlockProperties();
    return s_blockProperties[type].liquid;
}

// Get texture coordinates for a specific face of a block
void Block::getTextureCoords(BlockType type, BlockFace face, float& u, float& v) {
    if (!s_initialized) initBlockProperties();
    u = s_blockProperties[type].textureCoords[static_cast<size_t>(face)][0];
    v = s_blockProperties[type].textureCoords[static_cast<size_t>(face)][1];
}

// Get block name
const std::string& Block::getName(BlockType type) {
    if (!s_initialized) initBlockProperties();
    return s_blockProperties[type].name;
} 