#version 450

layout(location = 0) out vec3 vColor;

void main()
{
    vec2 positions[3] = vec2[](
        vec2( 0.0, -0.5),   // bottom
        vec2( 0.5,  0.5),   // right
        vec2(-0.5,  0.5)    // left
    );

    vec3 colors[3] = vec3[](
        vec3(1.0, 0.0, 0.0), // red
        vec3(0.0, 1.0, 0.0), // green
        vec3(0.0, 0.0, 1.0)  // blue
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    vColor = colors[gl_VertexIndex];
}