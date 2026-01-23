#version 450
#extension GL_EXT_nonuniform_qualifier : require

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

// Transform data structure
struct TransformData {
    float offsetX;
    float offsetY;
    float scale;
    float padding;
};

// Array of transforms (bindless)
layout(set = 0, binding = 3) readonly buffer TransformBuffer {
    TransformData transforms[];
};

// Push constants for selecting which resources to use
layout(push_constant) uniform PushConstants {
    uint textureIndex;
    uint transformIndex;
} pc;

// Output to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragTextureIndex;

void main() {
    // Fetch vertex index from index buffer
    uint vertexIndex = indices[gl_VertexIndex];

    // Fetch vertex data
    vec2 position = positions[vertexIndex];
    vec2 uv = UV[vertexIndex];

    // Fetch transform using push constant index (non-uniform access)
    TransformData transform = transforms[nonuniformEXT(pc.transformIndex)];

    // Apply transform
    vec2 transformedPos = position * transform.scale;
    transformedPos.x += transform.offsetX;
    transformedPos.y += transform.offsetY;

    gl_Position = vec4(transformedPos, 0.0, 1.0);
    fragTexCoord = uv;
    fragTextureIndex = pc.textureIndex;
}