#version 450
#extension GL_EXT_scalar_block_layout : require

// Storage buffer containing vertex positions (vec3)
layout(scalar, set = 0, binding = 0) readonly buffer PositionBuffer {
    vec3 positions[];
};

// Storage buffer containing vertex normals (vec3)
layout(scalar, set = 0, binding = 1) readonly buffer NormalBuffer {
    vec3 normals[];
};

// Storage buffer containing vertex colors (vec3)
layout(scalar, set = 0, binding = 2) readonly buffer ColorBuffer {
    vec3 colors[];
};

// Storage buffer containing indices
layout(scalar, set = 0, binding = 3) readonly buffer IndexBuffer {
    uint indices[];
};

// Transformation matrices
layout(set = 0, binding = 4) uniform TransformUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
    vec3 lightPos;
    float padding;
    vec3 viewPos;
} transform;

// Outputs to fragment shader
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec4 fragPosLightSpace;

void main() {
    uint vertexIndex = indices[gl_VertexIndex];

    vec3 position = positions[vertexIndex];
    vec3 normal = normals[vertexIndex];
    vec3 color = colors[vertexIndex];

    // World space position
    vec4 worldPos = transform.model * vec4(position, 1.0);
    fragPos = worldPos.xyz;

    // Transform normal to world space
    fragNormal = mat3(transpose(inverse(transform.model))) * normal;

    // Pass color through
    fragColor = color;

    // Position in light space for shadow mapping
    fragPosLightSpace = transform.lightSpaceMatrix * worldPos;

    // Final clip space position
    gl_Position = transform.projection * transform.view * worldPos;
}
