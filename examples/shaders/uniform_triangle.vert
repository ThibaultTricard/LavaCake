#version 450

// Output color to fragment shader
layout(location = 0) out vec3 fragColor;

// Hardcoded triangle vertices
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = vec3(1.0); // Pass white, fragment shader will use uniform color
}
