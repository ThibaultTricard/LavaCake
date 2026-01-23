#version 450

// Input from vertex shader
layout(location = 0) in vec3 fragColor;

// Uniform buffer with color data
layout(binding = 0) uniform ColorUBO {
    float r;
    float g;
    float b;
    float time;
} ubo;

// Output color
layout(location = 0) out vec4 outColor;

void main() {
    // Use the uniform buffer color
    outColor = vec4(ubo.r, ubo.g, ubo.b, 1.0);
}
