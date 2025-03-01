#include "VoxelRenderer.h"
#include "../Window.h"
#include "../Renderer/MetalRenderer.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>

#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>

// Uniform buffer for transformation matrices
struct Uniforms {
    simd::float4x4 modelMatrix;
    simd::float4x4 viewMatrix;
    simd::float4x4 projectionMatrix;
};

// Chunk mesh data
struct ChunkMeshData {
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> indexBuffer;
    uint32_t indexCount;
};

// Implementation details for voxel renderer
struct VoxelRenderer::Impl {
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLLibrary> library;
    id<MTLRenderPipelineState> pipelineState;
    id<MTLDepthStencilState> depthStencilState;
    id<MTLTexture> textureAtlas;
    id<MTLSamplerState> samplerState;
    id<MTLRenderCommandEncoder> currentRenderEncoder;
    
    // Chunk meshes
    std::unordered_map<ChunkPosition, ChunkMeshData, ChunkPosition::Hash> chunkMeshes;
};

// Helper function to convert glm::mat4 to simd::float4x4
simd::float4x4 glmToSIMD(const glm::mat4& matrix) {
    simd::float4x4 result;
    
    // Create each column vector
    simd::float4 col0 = {matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0]};
    simd::float4 col1 = {matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1]};
    simd::float4 col2 = {matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2]};
    simd::float4 col3 = {matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]};
    
    // Assign to columns
    result.columns[0] = col0;
    result.columns[1] = col1;
    result.columns[2] = col2;
    result.columns[3] = col3;
    
    return result;
}

// Constructor
VoxelRenderer::VoxelRenderer(Window* window)
    : m_window(window), m_impl(new Impl()) {
}

// Destructor
VoxelRenderer::~VoxelRenderer() {
    delete m_impl;
}

// Initialize the renderer
bool VoxelRenderer::init() {
    // Get Metal device from MetalRenderer
    // In a real implementation, we would get this from the MetalRenderer
    m_impl->device = MTLCreateSystemDefaultDevice();
    if (!m_impl->device) {
        std::cerr << "Failed to create Metal device!" << std::endl;
        return false;
    }
    
    // Create command queue
    m_impl->commandQueue = [m_impl->device newCommandQueue];
    if (!m_impl->commandQueue) {
        std::cerr << "Failed to create command queue!" << std::endl;
        return false;
    }
    
    // Load Metal library from file
    NSString* libraryPath = [NSString stringWithUTF8String:"VoxelShaders.metallib"];
    NSError* error = nil;
    m_impl->library = [m_impl->device newLibraryWithFile:libraryPath error:&error];
    
    if (!m_impl->library) {
        std::cerr << "Failed to load Metal library from file: " << [error.localizedDescription UTF8String] << std::endl;
        
        // Try to load default library as fallback
        m_impl->library = [m_impl->device newDefaultLibrary];
        if (!m_impl->library) {
            std::cerr << "Failed to load default Metal library!" << std::endl;
            return false;
        }
    }
    
    // Create render pipeline state
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    
    // Set vertex and fragment functions
    id<MTLFunction> vertexFunction = [m_impl->library newFunctionWithName:@"voxelVertexShader"];
    id<MTLFunction> fragmentFunction = [m_impl->library newFunctionWithName:@"voxelFragmentShader"];
    
    if (!vertexFunction || !fragmentFunction) {
        std::cerr << "Failed to load shader functions!" << std::endl;
        return false;
    }
    
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    
    // Set pixel format
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    // Create vertex descriptor
    MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    
    // Position attribute
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[0].offset = offsetof(ChunkVertex, position);
    vertexDescriptor.attributes[0].bufferIndex = 0;
    
    // Texture coordinate attribute
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].offset = offsetof(ChunkVertex, texCoord);
    vertexDescriptor.attributes[1].bufferIndex = 0;
    
    // Normal attribute
    vertexDescriptor.attributes[2].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[2].offset = offsetof(ChunkVertex, normal);
    vertexDescriptor.attributes[2].bufferIndex = 0;
    
    // Color attribute
    vertexDescriptor.attributes[3].format = MTLVertexFormatFloat4;
    vertexDescriptor.attributes[3].offset = offsetof(ChunkVertex, color);
    vertexDescriptor.attributes[3].bufferIndex = 0;
    
    // Buffer layout
    vertexDescriptor.layouts[0].stride = sizeof(ChunkVertex);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    
    // Create pipeline state
    error = nil;
    m_impl->pipelineState = [m_impl->device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    
    if (!m_impl->pipelineState) {
        std::cerr << "Failed to create render pipeline state: " << [error.localizedDescription UTF8String] << std::endl;
        return false;
    }
    
    // Create depth stencil state
    MTLDepthStencilDescriptor* depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    
    m_impl->depthStencilState = [m_impl->device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    
    // Load texture atlas
    if (!loadTextureAtlas()) {
        std::cerr << "Failed to load texture atlas!" << std::endl;
        return false;
    }
    
    // Create sampler state
    MTLSamplerDescriptor* samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    
    m_impl->samplerState = [m_impl->device newSamplerStateWithDescriptor:samplerDescriptor];
    
    return true;
}

// Render the world
void VoxelRenderer::render(World* world, const Camera& camera) {
    // Get Metal layer
    NSWindow* nsWindow = (__bridge NSWindow*)m_window->getNativeWindow();
    NSView* contentView = [nsWindow contentView];
    
    // Make sure the view's layer is a CAMetalLayer
    if (![contentView.layer isKindOfClass:[CAMetalLayer class]]) {
        // Set the view to use a metal layer
        [contentView setWantsLayer:YES];
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        metalLayer.device = m_impl->device;
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.framebufferOnly = YES;
        contentView.layer = metalLayer;
    }
    
    CAMetalLayer* metalLayer = (CAMetalLayer*)contentView.layer;
    
    // Get drawable
    id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
    if (!drawable) {
        std::cerr << "Failed to get next drawable" << std::endl;
        return;
    }
    
    // Create render pass descriptor
    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.2, 0.3, 0.8, 1.0);
    
    // Create depth texture
    MTLTextureDescriptor* depthTextureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                     width:drawable.texture.width
                                                                                                    height:drawable.texture.height
                                                                                                 mipmapped:NO];
    depthTextureDescriptor.usage = MTLTextureUsageRenderTarget;
    depthTextureDescriptor.storageMode = MTLStorageModePrivate;
    
    id<MTLTexture> depthTexture = [m_impl->device newTextureWithDescriptor:depthTextureDescriptor];
    
    renderPassDescriptor.depthAttachment.texture = depthTexture;
    renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    renderPassDescriptor.depthAttachment.clearDepth = 1.0;
    
    // Create command buffer
    id<MTLCommandBuffer> commandBuffer = [m_impl->commandQueue commandBuffer];
    
    // Create render command encoder
    m_impl->currentRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [m_impl->currentRenderEncoder setRenderPipelineState:m_impl->pipelineState];
    [m_impl->currentRenderEncoder setDepthStencilState:m_impl->depthStencilState];
    
    // Set texture and sampler
    [m_impl->currentRenderEncoder setFragmentTexture:m_impl->textureAtlas atIndex:0];
    [m_impl->currentRenderEncoder setFragmentSamplerState:m_impl->samplerState atIndex:0];
    
    // Get view and projection matrices
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();
    
    // Convert to simd matrices
    simd::float4x4 simdViewMatrix = glmToSIMD(viewMatrix);
    simd::float4x4 simdProjectionMatrix = glmToSIMD(projectionMatrix);
    
    // Render chunks
    for (const auto& pair : world->getChunks()) {
        Chunk* chunk = pair.second;
        renderChunk(chunk, camera);
    }
    
    // End encoding and present
    [m_impl->currentRenderEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

// Update chunk meshes
void VoxelRenderer::updateChunkMeshes(World* world) {
    // Get dirty chunks
    std::vector<Chunk*> dirtyChunks = world->getDirtyChunks();
    
    // Update meshes
    for (Chunk* chunk : dirtyChunks) {
        // Generate mesh
        chunk->generateMesh();
        
        // Create mesh data
        createChunkMesh(chunk);
        
        // Mark chunk as clean
        chunk->setDirty(false);
    }
}

// Create chunk mesh
void VoxelRenderer::createChunkMesh(Chunk* chunk) {
    // Get chunk position
    ChunkPosition position = chunk->getPosition();
    
    // Get vertices and indices
    const std::vector<ChunkVertex>& vertices = chunk->getVertices();
    const std::vector<uint32_t>& indices = chunk->getIndices();
    
    // Skip if empty
    if (vertices.empty() || indices.empty()) {
        return;
    }
    
    // Create vertex buffer
    id<MTLBuffer> vertexBuffer = [m_impl->device newBufferWithBytes:vertices.data()
                                                             length:vertices.size() * sizeof(ChunkVertex)
                                                            options:MTLResourceStorageModeShared];
    
    // Create index buffer
    id<MTLBuffer> indexBuffer = [m_impl->device newBufferWithBytes:indices.data()
                                                            length:indices.size() * sizeof(uint32_t)
                                                           options:MTLResourceStorageModeShared];
    
    // Store mesh data
    ChunkMeshData meshData;
    meshData.vertexBuffer = vertexBuffer;
    meshData.indexBuffer = indexBuffer;
    meshData.indexCount = static_cast<uint32_t>(indices.size());
    
    m_impl->chunkMeshes[position] = meshData;
}

// Render chunk
void VoxelRenderer::renderChunk(Chunk* chunk, const Camera& camera) {
    // Get chunk position
    ChunkPosition position = chunk->getPosition();
    
    // Check if mesh exists
    auto it = m_impl->chunkMeshes.find(position);
    if (it == m_impl->chunkMeshes.end()) {
        return;
    }
    
    // Get mesh data
    const ChunkMeshData& meshData = it->second;
    
    // Skip if empty
    if (meshData.indexCount == 0) {
        return;
    }
    
    // Calculate model matrix
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x * CHUNK_SIZE, 0.0f, position.z * CHUNK_SIZE));
    
    // Convert to simd matrix
    simd::float4x4 simdModelMatrix = glmToSIMD(modelMatrix);
    
    // Get view and projection matrices
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();
    
    // Convert to simd matrices
    simd::float4x4 simdViewMatrix = glmToSIMD(viewMatrix);
    simd::float4x4 simdProjectionMatrix = glmToSIMD(projectionMatrix);
    
    // Create uniforms
    Uniforms uniforms;
    uniforms.modelMatrix = simdModelMatrix;
    uniforms.viewMatrix = simdViewMatrix;
    uniforms.projectionMatrix = simdProjectionMatrix;
    
    // Set vertex buffer and uniforms
    [m_impl->currentRenderEncoder setVertexBuffer:meshData.vertexBuffer offset:0 atIndex:0];
    [m_impl->currentRenderEncoder setVertexBytes:&uniforms length:sizeof(Uniforms) atIndex:1];
    
    // Draw indexed primitives
    [m_impl->currentRenderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                             indexCount:meshData.indexCount
                                              indexType:MTLIndexTypeUInt32
                                            indexBuffer:meshData.indexBuffer
                                      indexBufferOffset:0];
}

// Load texture atlas
bool VoxelRenderer::loadTextureAtlas() {
    // In a real implementation, we would load a texture atlas from a file
    // For now, just create a simple texture with colors
    
    // Create texture descriptor
    MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                 width:256
                                                                                                height:256
                                                                                             mipmapped:YES];
    
    // Create texture
    m_impl->textureAtlas = [m_impl->device newTextureWithDescriptor:textureDescriptor];
    
    // Create texture data
    uint32_t textureData[256 * 256];
    
    // Fill texture with colors
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            uint32_t color;
            
            // Grass block
            if (x < 64 && y < 64) {
                // Grass side
                color = 0xFF7F5F3F; // Brown
            } else if (x < 64 && y < 128) {
                // Grass top
                color = 0xFF00FF00; // Green
            } else if (x < 128 && y < 64) {
                // Dirt
                color = 0xFF7F3F1F; // Dark brown
            } else if (x < 128 && y < 128) {
                // Stone
                color = 0xFF7F7F7F; // Gray
            } else if (x < 192 && y < 64) {
                // Sand
                color = 0xFFFFFF00; // Yellow
            } else if (x < 192 && y < 128) {
                // Water
                color = 0x7F0000FF; // Semi-transparent blue
            } else if (x < 256 && y < 64) {
                // Wood
                color = 0xFF7F3F00; // Brown
            } else if (x < 256 && y < 128) {
                // Leaves
                color = 0x7F00FF00; // Semi-transparent green
            } else {
                // Default
                color = 0xFFFFFFFF; // White
            }
            
            textureData[y * 256 + x] = color;
        }
    }
    
    // Upload texture data
    MTLRegion region = MTLRegionMake2D(0, 0, 256, 256);
    [m_impl->textureAtlas replaceRegion:region mipmapLevel:0 withBytes:textureData bytesPerRow:256 * sizeof(uint32_t)];
    
    // Generate mipmaps
    id<MTLCommandBuffer> commandBuffer = [m_impl->commandQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:m_impl->textureAtlas];
    [blitEncoder endEncoding];
    [commandBuffer commit];
    
    return true;
} 