#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

// Block type enum
enum class BlockType : uint8_t {
    Air = 0,
    Grass,
    Dirt,
    Stone,
    Sand,
    Water,
    Wood,
    Leaves,
    // Add more block types as needed
    Count
};

// Block face enum
enum class BlockFace : uint8_t {
    Front = 0,
    Back,
    Left,
    Right,
    Top,
    Bottom,
    Count
};

// Block class
class Block {
public:
    // Get block properties
    static bool isTransparent(BlockType type);
    static bool isSolid(BlockType type);
    static bool isLiquid(BlockType type);
    
    // Get texture coordinates for a specific face of a block
    static void getTextureCoords(BlockType type, BlockFace face, float& u, float& v);
    
    // Get block name
    static const std::string& getName(BlockType type);
    
private:
    // Block properties
    struct BlockProperties {
        bool transparent;
        bool solid;
        bool liquid;
        std::string name;
        // Texture coordinates for each face [face][u,v]
        float textureCoords[static_cast<size_t>(BlockFace::Count)][2];
    };
    
    // Block properties map
    static std::unordered_map<BlockType, BlockProperties> s_blockProperties;
    
    // Initialize block properties
    static void initBlockProperties();
    static bool s_initialized;
}; 