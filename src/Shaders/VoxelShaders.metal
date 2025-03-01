#include <metal_stdlib>
using namespace metal;

// Vertex input structure
struct VertexInput {
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float3 normal [[attribute(2)]];
    float4 color [[attribute(3)]];
};

// Vertex output structure
struct VertexOutput {
    float4 position [[position]];
    float2 texCoord;
    float3 normal;
    float4 color;
};

// Uniform buffer
struct Uniforms {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

// Vertex shader
vertex VertexOutput voxelVertexShader(VertexInput in [[stage_in]],
                                     constant Uniforms& uniforms [[buffer(1)]]) {
    VertexOutput out;
    
    // Transform position
    float4 worldPosition = uniforms.modelMatrix * float4(in.position, 1.0);
    float4 viewPosition = uniforms.viewMatrix * worldPosition;
    out.position = uniforms.projectionMatrix * viewPosition;
    
    // Pass through texture coordinates
    out.texCoord = in.texCoord;
    
    // Transform normal
    out.normal = normalize((uniforms.modelMatrix * float4(in.normal, 0.0)).xyz);
    
    // Pass through color
    out.color = in.color;
    
    return out;
}

// Fragment shader
fragment float4 voxelFragmentShader(VertexOutput in [[stage_in]],
                                   texture2d<float> textureAtlas [[texture(0)]],
                                   sampler textureSampler [[sampler(0)]]) {
    // Sample texture
    float4 textureColor = textureAtlas.sample(textureSampler, in.texCoord);
    
    // Simple lighting
    float3 lightDirection = normalize(float3(0.5, 1.0, 0.5));
    float diffuse = max(dot(in.normal, lightDirection), 0.2);
    
    // Combine texture color, vertex color, and lighting
    float4 finalColor = textureColor * in.color * diffuse;
    
    return finalColor;
} 