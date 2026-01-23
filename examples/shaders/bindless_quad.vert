#version 450
#extension GL_EXT_scalar_block_layout : require

// Storage buffer containing vertex positions
layout(scalar, set = 0, binding = 0) readonly buffer PositionBuffer {
    vec2 positions[];
};

// Storage buffer containing vertex colors (scalar layout = tightly packed, no vec3 padding)
layout(scalar, set = 0, binding = 1) readonly buffer ColorBuffer {
    vec3 colors[];
};

// Storage buffer containing indices
layout(scalar, set = 0, binding = 2) readonly buffer IndexBuffer {
    uint indices[];
};

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    // Use gl_VertexIndex to fetch the actual vertex index from the index buffer
    uint vertexIndex = indices[gl_VertexIndex];

    // Fetch vertex data using the index
    vec2 position = positions[vertexIndex];
    vec3 color = colors[vertexIndex];

    gl_Position = vec4(position, 0.0, 1.0);
    fragColor = color;
}
