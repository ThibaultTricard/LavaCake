#version 450
#extension GL_EXT_scalar_block_layout : require

#define MAX_LIGHTS 4

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

// Light data structure
struct Light {
    vec4 position;      // xyz = position, w = intensity
    vec4 color;         // rgb = color, a = radius
    mat4 lightSpaceMatrix;
};

// Transformation matrices and lights
layout(set = 0, binding = 4) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPos;
    uint numLights;
    Light lights[MAX_LIGHTS];
} scene;

// Outputs to fragment shader
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec4 fragPosLightSpace[MAX_LIGHTS];

void main() {
    uint vertexIndex = indices[gl_VertexIndex];

    vec3 position = positions[vertexIndex];
    vec3 normal = normals[vertexIndex];
    vec3 color = colors[vertexIndex];

    // World space position
    vec4 worldPos = scene.model * vec4(position, 1.0);
    fragPos = worldPos.xyz;

    // Transform normal to world space
    fragNormal = mat3(transpose(inverse(scene.model))) * normal;

    // Pass color through
    fragColor = color;

    // Position in light space for each light
    for (uint i = 0; i < MAX_LIGHTS; i++) {
        fragPosLightSpace[i] = scene.lights[i].lightSpaceMatrix * worldPos;
    }

    // Final clip space position
    gl_Position = scene.projection * scene.view * worldPos;
}
