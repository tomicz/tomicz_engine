#include <metal_stdlib>
using namespace metal;

// Vertex input structure
struct VertexInput {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

// Vertex output structure
struct VertexOutput {
    float4 position [[position]];
    float4 color;
};

// Vertex shader
vertex VertexOutput vertexShader(VertexInput in [[stage_in]]) {
    VertexOutput output;
    output.position = float4(in.position, 0.0, 1.0);
    output.color = in.color;
    return output;
}

// Fragment shader
fragment float4 fragmentShader(VertexOutput in [[stage_in]]) {
    return in.color;
} 