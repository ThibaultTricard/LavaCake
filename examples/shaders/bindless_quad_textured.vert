#version 450

// Storage buffer containing vertex positions
layout(set = 0, binding = 0) readonly buffer PositionBuffer {
    vec2 positions[];
};

// Storage buffer containing vertex texture coordinates
layout(set = 0, binding = 1) readonly buffer UVBuffer {
    vec2 UV[];
};

// Storage buffer containing indices
layout(set = 0, binding = 2) readonly buffer IndexBuffer {
    uint indices[];
};

// Output to fragment shader
layout(location = 0) out vec2 texCoord;

void main() {
    // Use gl_VertexIndex to fetch the actual vertex index from the index buffer
    uint vertexIndex = indices[gl_VertexIndex];

    // Fetch vertex data using the index
    vec2 position = positions[vertexIndex];
    vec2 uv = UV[vertexIndex];

    gl_Position = vec4(position, 0.0, 1.0);
    texCoord = uv;
}
