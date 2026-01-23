#version 450


// Input Checkerboard Texture
layout( set = 0, binding = 3) uniform sampler2D checkerTexture;

// Input color from vertex shader
layout(location = 0) in vec2 uv;

// Output color
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(checkerTexture,uv).xxx, 1.0);
}
