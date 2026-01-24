#version 450
#extension GL_EXT_scalar_block_layout : require

// Storage buffer containing vertex positions (vec3)
layout(scalar, set = 0, binding = 0) readonly buffer PositionBuffer {
    vec3 positions[];
};

// Storage buffer containing vertex colors (vec3)
layout(scalar, set = 0, binding = 1) readonly buffer ColorBuffer {
    vec3 colors[];
};

// Storage buffer containing indices
layout(scalar, set = 0, binding = 2) readonly buffer IndexBuffer {
    uint indices[];
};

// Uniform buffer for transformation matrices
layout(set = 0, binding = 3) uniform TransformUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    // Use gl_VertexIndex to fetch the actual vertex index from the index buffer
    uint vertexIndex = indices[gl_VertexIndex];

    // Fetch vertex data using the index
    vec3 position = positions[vertexIndex];
    vec3 color = colors[vertexIndex];

    // Apply MVP transformation
    gl_Position = transform.projection * transform.view * transform.model * vec4(position, 1.0);
    fragColor = color;
}
