#version 460
#extension GL_EXT_ray_tracing : require

// Ray payload - must match raygen/closesthit
struct RayPayload {
    vec3 hitPos;
    vec3 hitNormal;
    vec3 albedo;
    bool hit;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    payload.hit = false;
}
