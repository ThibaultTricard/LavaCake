#version 450
#extension GL_EXT_scalar_block_layout : require

// Storage buffer containing vertex positions (vec3)
layout(scalar, set = 0, binding = 0) readonly buffer PositionBuffer {
    vec3 positions[];
};

// Storage buffer containing indices
layout(scalar, set = 0, binding = 1) readonly buffer IndexBuffer {
    uint indices[];
};

// Light space matrix
layout(set = 0, binding = 2) uniform LightSpaceUBO {
    mat4 lightSpaceMatrix;
} light;

void main() {
    uint vertexIndex = indices[gl_VertexIndex];
    vec3 position = positions[vertexIndex];

    gl_Position = light.lightSpaceMatrix * vec4(position, 1.0);
}
