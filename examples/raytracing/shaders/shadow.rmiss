#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 1) rayPayloadInEXT bool isShadowed;

void main() {
    // Shadow ray didn't hit anything - point is NOT in shadow
    isShadowed = false;
}
