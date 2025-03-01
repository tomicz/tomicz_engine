#include "MetalRenderer.h"
#include "../Window.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <iostream>
#include <vector>

// Implementation details for Metal renderer
struct MetalRenderer::Impl {
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLLibrary> library;
    id<MTLRenderPipelineState> pipelineState;
    CAMetalLayer* metalLayer;
    
    // Vertex data for a square
    struct Vertex {
        float position[2];
        float color[4];
    };
    
    id<MTLBuffer> vertexBuffer;
};

MetalRenderer::MetalRenderer(Window* window)
    : m_window(window), m_impl(new Impl()) {
}

MetalRenderer::~MetalRenderer() {
    delete m_impl;
}

bool MetalRenderer::init() {
    // Get the Metal device
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
    
    // Set up Metal layer
    NSWindow* nsWindow = (__bridge NSWindow*)m_window->getNativeWindow();
    NSView* contentView = [nsWindow contentView];
    
    m_impl->metalLayer = [CAMetalLayer layer];
    m_impl->metalLayer.device = m_impl->device;
    m_impl->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    m_impl->metalLayer.framebufferOnly = YES;
    m_impl->metalLayer.frame = [contentView bounds];
    
    [contentView setWantsLayer:YES];
    [contentView setLayer:m_impl->metalLayer];
    
    // Create vertex buffer for square
    std::vector<Impl::Vertex> vertices = {
        // Position (x, y), Color (r, g, b, a)
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // Bottom left - red
        {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},  // Bottom right - green
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Top left - blue
        {{ 0.5f,  0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}   // Top right - yellow
    };
    
    m_impl->vertexBuffer = [m_impl->device newBufferWithBytes:vertices.data()
                                                       length:vertices.size() * sizeof(Impl::Vertex)
                                                      options:MTLResourceStorageModeShared];
    
    // Load shaders
    if (!loadShaders()) {
        return false;
    }
    
    return true;
}

bool MetalRenderer::loadShaders() {
    // Load default library from bundle
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* path = [bundle pathForResource:@"Shaders" ofType:@"metallib"];
    
    NSError* error = nil;
    
    // If we can't find the shader library in the bundle, try to load it from the current directory
    if (!path) {
        NSString* currentDir = [NSString stringWithUTF8String:getenv("PWD")];
        path = [currentDir stringByAppendingPathComponent:@"Shaders.metallib"];
    }
    
    // Use the non-deprecated API to load the library
    NSURL* url = [NSURL fileURLWithPath:path];
    m_impl->library = [m_impl->device newLibraryWithURL:url error:&error];
    
    if (!m_impl->library) {
        std::cerr << "Failed to load Metal library: " << [error.localizedDescription UTF8String] << std::endl;
        
        // Try to create a default library as a fallback
        m_impl->library = [m_impl->device newDefaultLibrary];
        if (!m_impl->library) {
            std::cerr << "Failed to create default Metal library!" << std::endl;
            return false;
        }
    }
    
    // Get shader functions
    id<MTLFunction> vertexFunction = [m_impl->library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [m_impl->library newFunctionWithName:@"fragmentShader"];
    
    if (!vertexFunction || !fragmentFunction) {
        std::cerr << "Failed to load shader functions!" << std::endl;
        return false;
    }
    
    // Create render pipeline descriptor
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = m_impl->metalLayer.pixelFormat;
    
    // Create vertex descriptor
    MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    
    // Position attribute
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = offsetof(Impl::Vertex, position);
    vertexDescriptor.attributes[0].bufferIndex = 0;
    
    // Color attribute
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
    vertexDescriptor.attributes[1].offset = offsetof(Impl::Vertex, color);
    vertexDescriptor.attributes[1].bufferIndex = 0;
    
    // Buffer layout
    vertexDescriptor.layouts[0].stride = sizeof(Impl::Vertex);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    
    // Create render pipeline state
    error = nil;
    m_impl->pipelineState = [m_impl->device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    
    if (!m_impl->pipelineState) {
        std::cerr << "Failed to create render pipeline state: " << [error.localizedDescription UTF8String] << std::endl;
        return false;
    }
    
    return true;
}

void MetalRenderer::render() {
    // Get the next drawable
    id<CAMetalDrawable> drawable = [m_impl->metalLayer nextDrawable];
    if (!drawable) {
        return;
    }
    
    // Create render pass descriptor
    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.2, 0.2, 0.2, 1.0);
    
    // Create command buffer
    id<MTLCommandBuffer> commandBuffer = [m_impl->commandQueue commandBuffer];
    
    // Create render command encoder
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder setRenderPipelineState:m_impl->pipelineState];
    [renderEncoder setVertexBuffer:m_impl->vertexBuffer offset:0 atIndex:0];
    
    // Draw the square (2 triangles)
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    
    // End encoding and present
    [renderEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
} 